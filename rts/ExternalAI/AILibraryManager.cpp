/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#include "AILibraryManager.h"

#include "Interface/aidefines.h"
#include "Interface/SAIInterfaceLibrary.h"
#include "AIInterfaceKey.h"
#include "AIInterfaceLibraryInfo.h"
#include "AIInterfaceLibrary.h"
#include "SkirmishAILibraryInfo.h"
#include "SkirmishAIData.h"
#include "SkirmishAIKey.h"

#include "System/StringUtil.h"
#include "System/Log/ILog.h"
#include "System/Platform/errorhandler.h"
#include "System/Platform/SharedLib.h"
#include "System/FileSystem/FileHandler.h"
#include "System/FileSystem/DataDirsAccess.h"
#include "System/FileSystem/FileQueryFlags.h"
#include "System/FileSystem/FileSystem.h"
#include "System/FileSystem/SimpleParser.h" // for Split()

#include <climits>
#include <cstdio>
#include <cstring>

#include <string>
#include <sstream>

static AILibraryManager aiLibraryManager;
static AILibraryManager* gAILibraryManager = nullptr;


AILibraryManager* AILibraryManager::GetInstance() {
	if (gAILibraryManager == nullptr) {
		gAILibraryManager = &aiLibraryManager;
		gAILibraryManager->Init();
	}

	return gAILibraryManager;
}

void AILibraryManager::Destroy() {
	gAILibraryManager->Kill();
	gAILibraryManager = nullptr;
}


void AILibraryManager::Init() {
	ClearAllInfos();
	GatherInterfaceLibrariesInfos();
	GatherSkirmishAIsLibrariesInfos();
}

void AILibraryManager::Kill() {
	ReleaseEverything();
	ClearAllInfos();
}

void AILibraryManager::ClearAllInfos() {
	interfaceInfos.clear();
	skirmishAIInfos.clear();

	interfaceKeys.clear();
	skirmishAIKeys.clear();

	duplicateInterfaceInfos.clear();
	duplicateSkirmishAIInfos.clear();

	assert(loadedAIInterfaceLibraries.empty());
}


void AILibraryManager::GatherInterfaceLibrariesInfos() {
	typedef std::vector<std::string> T_dirs;
	typedef std::map<const AIInterfaceKey, std::set<std::string> > T_dupInt;

	// cause we use CFileHandler for searching files,
	// we are automatically searching in all data-dirs

	// Read from AI Interface info files
	// we are looking for:
	// {AI_INTERFACES_DATA_DIR}/{*}/{*}/InterfaceInfo.lua
	T_dirs aiInterfaceDataDirs = std::move(dataDirsAccess.FindDirsInDirectSubDirs(AI_INTERFACES_DATA_DIR));
	T_dupInt duplicateInterfaceInfoCheck;

	for (const auto& possibleDataDir: aiInterfaceDataDirs) {
		const T_dirs& infoFiles = CFileHandler::FindFiles(possibleDataDir, "InterfaceInfo.lua");

		if (infoFiles.empty())
			continue;

		// interface info is available
		const std::string& infoFile = infoFiles[0];

		// generate and store the interface info
		CAIInterfaceLibraryInfo interfaceInfo = CAIInterfaceLibraryInfo(infoFile);

		interfaceInfo.SetDataDir(FileSystem::EnsureNoPathSepAtEnd(possibleDataDir));
		interfaceInfo.SetDataDirCommon(FileSystem::GetParent(possibleDataDir) + "common");

		const AIInterfaceKey interfaceKey = interfaceInfo.GetKey();

		interfaceKeys.insert(interfaceKey);

		// if no interface info with this key yet, store it
		if (interfaceInfos.find(interfaceKey) == interfaceInfos.end())
			interfaceInfos[interfaceKey] = interfaceInfo;

		// for debug-info, in case one interface is specified multiple times
		duplicateInterfaceInfoCheck[interfaceKey].insert(infoFile);
	}

	// filter out interfaces that are specified multiple times
	for (const auto& info: duplicateInterfaceInfoCheck) {
		if (info.second.size() < 2)
			continue;

		duplicateInterfaceInfos[info.first] = info.second;

		if (!LOG_IS_ENABLED(L_ERROR))
			continue;

		const std::string& isn = info.first.GetShortName();
		const std::string& iv =  info.first.GetVersion();
		const std::string* lastDir = nullptr;

		LOG_L(L_ERROR, "[%s] duplicate AI Interface Info found", __func__);
		LOG_L(L_ERROR, "\tfor interface %s %s", isn.c_str(), iv.c_str());
		LOG_L(L_ERROR, "\tin files");

		for (const std::string& dir: info.second) {
			LOG_L(L_ERROR, "\t%s", dir.c_str());
			lastDir = &dir;
		}

		LOG_L(L_ERROR, "\tusing dir %s", lastDir->c_str());
	}
}

void AILibraryManager::GatherSkirmishAIsLibrariesInfos() {
	T_dupSkirm duplicateSkirmishAIInfoCheck;

	GatherSkirmishAIsLibrariesInfosFromLuaFiles(duplicateSkirmishAIInfoCheck);
	GatherSkirmishAIsLibrariesInfosFromInterfaceLibrary(duplicateSkirmishAIInfoCheck);
	FilterDuplicateSkirmishAILibrariesInfos(duplicateSkirmishAIInfoCheck);
}

void AILibraryManager::StoreSkirmishAILibraryInfos(
	T_dupSkirm duplicateSkirmishAIInfoCheck,
	CSkirmishAILibraryInfo& skirmishAIInfo,
	const std::string& sourceDesc
) {
	skirmishAIInfo.SetLuaAI(false);

	const SkirmishAIKey aiKey = skirmishAIInfo.GetKey();
	const AIInterfaceKey interfaceKey = FindFittingInterfaceSpecifier(
		skirmishAIInfo.GetInterfaceShortName(),
		skirmishAIInfo.GetInterfaceVersion(),
		interfaceKeys
	);

	if (!interfaceKey.IsUnspecified()) {
		SkirmishAIKey skirmishAIKey = SkirmishAIKey(aiKey, interfaceKey);
		skirmishAIKeys.insert(skirmishAIKey);

		// if no AI info with this key yet, store it
		if (skirmishAIInfos.find(skirmishAIKey) == skirmishAIInfos.end())
			skirmishAIInfos[skirmishAIKey] = skirmishAIInfo;

		// for debug-info, in case one AI is specified multiple times
		duplicateSkirmishAIInfoCheck[skirmishAIKey].insert(sourceDesc);
		return;
	}

	const std::string& isn = skirmishAIInfo.GetShortName();
	const std::string& iv  = skirmishAIInfo.GetVersion();

	LOG_L(L_ERROR, "[%s] required AI Interface for Skirmish AI %s %s not found", __func__, isn.c_str(), iv.c_str());
}

void AILibraryManager::GatherSkirmishAIsLibrariesInfosFromLuaFiles(T_dupSkirm duplicateSkirmishAIInfoCheck) {
	typedef std::vector<std::string> T_dirs;

	// Read from Skirmish AI info and option files
	// we are looking for:
	// {SKIRMISH_AI_DATA_DIR}/{*}/{*}/AIInfo.lua
	// {SKIRMISH_AI_DATA_DIR}/{*}/{*}/AIOptions.lua
	T_dirs skirmishAIDataDirs = dataDirsAccess.FindDirsInDirectSubDirs(SKIRMISH_AI_DATA_DIR);

	for (const auto& possibleDataDir : skirmishAIDataDirs) {
		const T_dirs& infoFiles = CFileHandler::FindFiles(possibleDataDir, "AIInfo.lua");

		if (infoFiles.empty())
			continue;

		// skirmish AI info is available
		const std::string& infoFile = infoFiles[0];
		const T_dirs& optionFile = CFileHandler::FindFiles(possibleDataDir, "AIOptions.lua");

		std::string optionFileName;

		if (!optionFile.empty())
			optionFileName = optionFile[0];

		// generate and store the ai info
		CSkirmishAILibraryInfo skirmishAIInfo = CSkirmishAILibraryInfo(infoFile, optionFileName);

		skirmishAIInfo.SetDataDir(FileSystem::EnsureNoPathSepAtEnd(possibleDataDir));
		skirmishAIInfo.SetDataDirCommon(FileSystem::GetParent(possibleDataDir) + "common");

		StoreSkirmishAILibraryInfos(duplicateSkirmishAIInfoCheck, skirmishAIInfo, infoFile);
	}
}

void AILibraryManager::GatherSkirmishAIsLibrariesInfosFromInterfaceLibrary(T_dupSkirm duplicateSkirmishAIInfoCheck) {
	const T_interfaceInfos& intInfs = GetInterfaceInfos();

	for (const auto& intInf: intInfs) {
		// only try to lookup Skirmish AI infos through the Interface library
		// if it explicitly states support for this in InterfaceInfo.lua
		if (!intInf.second.IsLookupSupported())
			continue;

		const CAIInterfaceLibrary* intLib = FetchInterface(intInf.second.GetKey());
		const int aiCount = intLib->GetSkirmishAICount();

		for (int aii = 0; aii < aiCount; ++aii) {
			const std::map<std::string, std::string>& rawInfos = intLib->GetSkirmishAIInfos(aii);
			const std::string& rawLuaOptions = intLib->GetSkirmishAIOptions(aii);

			// generate and store the ai info
			//
			// NOTE We do not set the data-dir(s) for interface looked-up
			//   AIs. This is the duty of the AI Interface plugin.
			CSkirmishAILibraryInfo skirmishAIInfo = CSkirmishAILibraryInfo(rawInfos, rawLuaOptions);

			StoreSkirmishAILibraryInfos(duplicateSkirmishAIInfoCheck, skirmishAIInfo, intInf.first.ToString());
		}
	}
}

void AILibraryManager::FilterDuplicateSkirmishAILibrariesInfos(T_dupSkirm duplicateSkirmishAIInfoCheck) {

	// filter out skirmish AIs that are specified multiple times
	for (const auto& info: duplicateSkirmishAIInfoCheck) {
		if (info.second.size() < 2)
			continue;

		duplicateSkirmishAIInfos[info.first] = info.second;

		if (!LOG_IS_ENABLED(L_WARNING))
			continue;

		LOG_L(L_WARNING, "[%s] duplicate Skirmish AI Info found", __func__);
		LOG_L(L_WARNING, "\tfor Skirmish AI %s %s", info.first.GetShortName().c_str(), info.first.GetVersion().c_str());
		LOG_L(L_WARNING, "\tin files");

		const std::string* lastDir = nullptr;

		for (const std::string& dir: info.second) {
			LOG_L(L_WARNING, "\t%s", dir.c_str());
			lastDir = &dir;
		}

		LOG_L(L_WARNING, "\tusing dir %s", lastDir->c_str());
	}
}




std::vector<SkirmishAIKey> AILibraryManager::FittingSkirmishAIKeys(const SkirmishAIKey& skirmishAIKey) const {
	std::vector<SkirmishAIKey> matchedKeys;

	if (skirmishAIKey.IsUnspecified())
		return matchedKeys;

	matchedKeys.reserve(skirmishAIKeys.size());

	for (const auto& aiKey: skirmishAIKeys) {
		// check if the AI name matches
		if (skirmishAIKey.GetShortName() != aiKey.GetShortName())
			continue;

		// check if the AI version matches (if one is specified i.e. non-empty)
		if (!skirmishAIKey.GetVersion().empty() && skirmishAIKey.GetVersion() != aiKey.GetVersion())
			continue;

		matchedKeys.push_back(aiKey);
	}

	return matchedKeys;
}



const CSkirmishAILibrary* AILibraryManager::FetchSkirmishAILibrary(const SkirmishAIKey& skirmishAIKey) {
	const auto aiInfo = skirmishAIInfos.find(skirmishAIKey);

	if (aiInfo == skirmishAIInfos.end()) {
		const std::string& ksn = skirmishAIKey.GetShortName();
		const std::string& kv  = skirmishAIKey.GetVersion();

		LOG_L(L_ERROR, "[%s] unknown skirmish AI %s %s specified", __func__, ksn.c_str(), kv.c_str());
		return nullptr;
	}

	CAIInterfaceLibrary* intLib = FetchInterface(skirmishAIKey.GetInterface());
	const CSkirmishAILibrary* aiLib = nullptr;

	if ((intLib != nullptr) && intLib->IsInitialized())
		aiLib = intLib->FetchSkirmishAILibrary(aiInfo->second);

	return aiLib;
}

void AILibraryManager::ReleaseSkirmishAILibrary(const SkirmishAIKey& skirmishAIKey) {
	CAIInterfaceLibrary* intLib = FetchInterface(skirmishAIKey.GetInterface());

	// do not release if the AI Interface (and hence the AI) was not initialized
	if ((intLib == nullptr) || !intLib->IsInitialized())
		return;

	intLib->ReleaseSkirmishAILibrary(skirmishAIKey);
	// only releases the library if its load count is 0
	ReleaseInterface(skirmishAIKey.GetInterface());
}


void AILibraryManager::ReleaseEverything() {
	for (const auto& p: loadedAIInterfaceLibraries) {
		CAIInterfaceLibrary* intLib = FetchInterface(p.first);

		if ((intLib == nullptr) || !intLib->IsInitialized())
			continue;

		intLib->ReleaseAllSkirmishAILibraries();
		// only releases the library if its load count is 0
		ReleaseInterface(p.first);
	}
}



CAIInterfaceLibrary* AILibraryManager::FetchInterface(const AIInterfaceKey& interfaceKey) {
	const auto interfacePos = loadedAIInterfaceLibraries.find(interfaceKey);

	if (interfacePos != loadedAIInterfaceLibraries.end())
		return (interfacePos->second).get();

	// interface not yet loaded
	const auto interfaceInfo = interfaceInfos.find(interfaceKey);

	if (interfaceInfo == interfaceInfos.end())
		return nullptr;

	// storing this for later use, even if it failed to init
	std::unique_ptr<CAIInterfaceLibrary>& ptr = loadedAIInterfaceLibraries[interfaceKey];

	ptr.reset(new CAIInterfaceLibrary(interfaceInfo->second));

	if (!ptr->IsInitialized())
		ptr.reset();

	return (ptr.get());
}

void AILibraryManager::ReleaseInterface(const AIInterfaceKey& interfaceKey) {
	const auto interfacePos = loadedAIInterfaceLibraries.find(interfaceKey);

	if (interfacePos == loadedAIInterfaceLibraries.end())
		return;

	CAIInterfaceLibrary* interfaceLib = (interfacePos->second).get();

	if (interfaceLib->GetLoadCount() != 0)
		return;

	loadedAIInterfaceLibraries.erase(interfacePos);
}


AIInterfaceKey AILibraryManager::FindFittingInterfaceSpecifier(
	const std::string& shortName,
	const std::string& minVersion,
	const AILibraryManager::T_interfaceSpecs& keys
) {
	int minDiff = INT_MAX;
	AIInterfaceKey fittingKey = AIInterfaceKey(); // unspecified key

	for (const auto& key: keys) {
		if (shortName != key.GetShortName())
			continue;

		const int diff = AILibraryManager::VersionCompare(key.GetVersion(), minVersion);

		if (diff < 0 || diff >= minDiff)
			continue;

		fittingKey = key;
		minDiff = diff;
	}

	return fittingKey;
}




void AILibraryManager::OutputAIInterfacesInfo() {
	const AILibraryManager* libMan = AILibraryManager::GetInstance();
	const T_interfaceSpecs& intKeys = libMan->GetInterfaceKeys();

	printf("#\n");
	printf("# Available Spring Skirmish AIs\n");
	printf("# -----------------------------\n");
	printf("# %-20s %s\n", "[Name]", "[Version]");

	for (const auto& key: intKeys) {
		printf("  %-20s %s\n", key.GetShortName().c_str(), key.GetVersion().c_str());
	}

	printf("#\n");
}

SkirmishAIKey AILibraryManager::ResolveSkirmishAIKey(const SkirmishAIKey& skirmishAIKey) const {
	std::vector<SkirmishAIKey> fittingKeys = std::move(FittingSkirmishAIKeys(skirmishAIKey));

	if (fittingKeys.empty())
		return SkirmishAIKey();

	// look for the one with the highest version number,
	// in case there are multiple fitting ones.
	size_t bestIndex = 0;

	for (size_t k = 1; k < fittingKeys.size(); ++k) {
		if (AILibraryManager::VersionCompare(fittingKeys[k].GetVersion(), fittingKeys[bestIndex].GetVersion()) <= 0)
			continue;

		bestIndex = k;
	}

	return fittingKeys[bestIndex];
}

void AILibraryManager::OutputSkirmishAIInfo() {
	const AILibraryManager* libMan = AILibraryManager::GetInstance();
	const T_skirmishAIKeys& aiKeys = libMan->GetSkirmishAIKeys();

	printf("# Available Spring Skirmish AIs\n");
	printf("# -----------------------------\n");
	printf("# %-20s %-20s %-20s %s\n", "[Name]", "[Version]", "[Interface-name]", "[Interface-version]");

	for (const auto& key: aiKeys) {
		const std::string& ksn = key.GetShortName();
		const std::string& kv  = key.GetVersion();
		const std::string& isn = (key.GetInterface()).GetShortName();
		const std::string& iv  = (key.GetInterface()).GetVersion();

		printf("# %-20s %-20s %-20s %s\n", ksn.c_str(), kv.c_str(), isn.c_str(), iv.c_str());
	}

	const T_dupSkirm& duplicateSkirmishAIInfos = libMan->GetDuplicateSkirmishAIInfos();

	for (const auto& info: duplicateSkirmishAIInfos) {
		const std::string& isn = info.first.GetShortName();
		const std::string& iv  = info.first.GetVersion();

		printf("# WARNING: Duplicate Skirmish AI Info found:\n");
		printf("# \tfor Skirmish AI: %s %s\n", isn.c_str(), iv.c_str());
		printf("# \tin files:\n");

		for (const auto& dir: info.second) {
			printf("# \t%s\n", dir.c_str());
		}
	}

	printf("#\n");
}

int AILibraryManager::VersionCompare(const std::string& version1, const std::string& version2)
{
	const std::vector<std::string>& parts1 = CSimpleParser::Split(version1, ".");
	const std::vector<std::string>& parts2 = CSimpleParser::Split(version2, ".");

	const unsigned int maxParts = (parts1.size() > parts2.size())? parts1.size(): parts2.size();

	int diff = 0;
	for (unsigned int i = 0; i < maxParts; ++i) {
		const std::string& v1p = (i < parts1.size())? parts1[i] : "0";
		const std::string& v2p = (i < parts2.size())? parts2[i] : "0";
		diff += (1 << ((maxParts - i) * 2)) * v1p.compare(v2p);
	}

	// compute the sign of diff -> 1, 0 or -1
	return (diff != 0) | -(int)((unsigned int)((int)diff) >> (sizeof(int) * CHAR_BIT - 1));
}

