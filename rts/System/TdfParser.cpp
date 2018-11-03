/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#include <algorithm>
#include <cctype>
#include <climits>
#include <stdexcept>
#include <sstream>
#include <vector>


#include "Lua/LuaParser.h"
#include "System/TdfParser.h"
#include "System/StringUtil.h"
#include "System/FileSystem/ArchiveScanner.h"
#include "System/FileSystem/FileHandler.h"
#include "System/FileSystem/VFSHandler.h"
#include "System/Log/ILog.h"

void TdfParser::TdfSection::print(std::ostream & out) const
{
	for (const auto& section: sections) {
		out << "[" << section.first << "]\n{\n";
		section.second->print(out);
		out << "}\n";
	}
	for (const auto& value: values) {
		out << value.first << "=" << value.second << ";\n";
	}
}

TdfParser::TdfSection* TdfParser::TdfSection::construct_subsection(const std::string& name)
{
	std::string lowerd_name = StringToLower(name);
	const auto it = sections.find(lowerd_name);

	if (it != sections.end())
		return it->second;

	TdfSection* ret = new TdfSection;
	sections[lowerd_name] = ret;
	return ret;
}

bool TdfParser::TdfSection::remove(const std::string& key, bool caseSensitive)
{
	bool ret = false;

	if (caseSensitive) {
		valueMap_t::iterator it = values.find(key);
		if ((ret = (it != values.end()))) {
			values.erase(it);
		}
	} else {
		// don't assume <key> is already in lowercase
		const std::string lowerKey = StringToLower(key);
		for (valueMap_t::iterator it = values.begin(); it != values.end(); ) {
			if (StringToLower(it->first) == lowerKey) {
				it = values.erase(it);
				ret = true;
			} else {
				++it;
			}
		}
	}

	return ret;
}

void TdfParser::TdfSection::add_name_value(const std::string& name, const std::string& value)
{
	std::string lowerd_name = StringToLower(name);
	values[lowerd_name] = value;
}

TdfParser::TdfSection::~TdfSection()
{
	for (auto& section: sections) {
		delete section.second;
	}
}



TdfParser::TdfParser(char const* buf, size_t size)
{
	LoadBuffer(buf, size);
}

TdfParser::TdfParser(std::string const& filename)
{
	LoadFile(filename);
}

TdfParser::~TdfParser() = default;

void TdfParser::print(std::ostream & out) const {
	root_section.print(out);
}


void TdfParser::ParseLuaTable(const LuaTable& table, TdfSection* currentSection) {
	std::vector<std::string> keys;
	table.GetKeys(keys);
	for (const std::string& key: keys) {
		LuaTable::DataType dt = table.GetType(key);
		switch (dt) {
			case LuaTable::DataType::TABLE: {
				ParseLuaTable(table.SubTable(key), currentSection->construct_subsection(key));
				break;
			}
			case LuaTable::DataType::BOOLEAN: {
				currentSection->AddPair(key, table.Get(key, false));
				break;
			}
			case LuaTable::DataType::NUMBER: {
				currentSection->AddPair(key, table.Get(key, 0.0f));
				break;
			}
			case LuaTable::DataType::STRING: {
				currentSection->AddPair(key, table.Get(key, std::string("")));
				break;
			}
			default:
				throw content_error("invalid datatype for key " + key);
		}
	}
}


void TdfParser::ParseBuffer(char const* buf, size_t size) {
	CVFSHandler* oldHandler = vfsHandler;
	CVFSHandler  tmpHandler;

	// block other threads from getting the global until we are done
	CVFSHandler::GrabLock();
	CVFSHandler::SetGlobalInstanceRaw(&tmpHandler);
	tmpHandler.AddArchive(CArchiveScanner::GetSpringBaseContentName(), false);

	{
		const std::string script = std::string("local TDF = VFS.Include('gamedata/parse_tdf.lua'); return TDF.ParseText([[") + buf + "]])";

		LuaParser luaParser(script, SPRING_VFS_BASE);
		luaParser.Execute();
		ParseLuaTable(luaParser.GetRoot(), GetRootSection());
	}

	CVFSHandler::SetGlobalInstanceRaw(oldHandler);
	CVFSHandler::FreeLock();
}

void TdfParser::LoadBuffer(char const* buf, size_t size)
{
	this->filename = "buffer";
	ParseBuffer(buf, size);
}


void TdfParser::LoadFile(std::string const& filename)
{

	this->filename = filename;
	CFileHandler file(filename);
	if (!file.FileExists()) {
		throw content_error("file " + filename + " not found");
	}

	const size_t fileBuf_size = file.FileSize();
	//char* fileBuf = new char[fileBuf_size];
	std::vector<char> fileBuf(fileBuf_size);

	file.Read(fileBuf.data(), file.FileSize());
	ParseBuffer(fileBuf.data(), fileBuf_size);

	//delete[] fileBuf;
}


std::string TdfParser::SGetValueDef(std::string const& defaultValue, std::string const& location) const
{
	std::string lowerd = StringToLower(location);
	std::string value;
	bool found = SGetValue(value, lowerd);
	if (!found) {
		value = defaultValue;
	}
	return value;
}

bool TdfParser::SGetValue(std::string &value, std::string const& location) const
{
	std::string lowerd = StringToLower(location);
	std::string searchpath; // for error-messages
	// split the location string
	const std::vector<std::string>& loclist = GetLocationVector(lowerd);
	sectionsMap_t::const_iterator sit = root_section.sections.find(loclist[0]);
	if (sit == root_section.sections.end()) {
		value = "Section " + loclist[0] + " missing in file " + filename;
		return false;
	}
	TdfSection* sectionptr = sit->second;
	searchpath = loclist[0];
	for (unsigned int i=1; i < loclist.size()-1; ++i) {
		//const char *arg = loclist[i].c_str();
		searchpath += '\\';
		searchpath += loclist[i];
		sit = sectionptr->sections.find(loclist[i]);
		if (sit == sectionptr->sections.end()) {
			value = "Section " + searchpath + " missing in file " + filename;
			return false;
		}
		sectionptr = sit->second;
	}
	searchpath += '\\';
	searchpath += loclist[loclist.size()-1];

	valueMap_t::const_iterator vit =
		sectionptr->values.find(loclist[loclist.size()-1]);
	if (vit == sectionptr->values.end()) {
		value = "Value " + searchpath + " missing in file " + filename;
		return false;
	}
	std::string svalue = vit->second;
	value = svalue;
	return true;
}

bool TdfParser::GetValue(bool& val, const std::string& location) const
{
	std::string buf;
	if (SGetValue(buf, location)) {
		int tempval;
		std::istringstream stream(buf);
		stream >> tempval;
		val = (tempval != 0);
		return true;
	} else {
		return false;
	}
}

const TdfParser::valueMap_t& TdfParser::GetAllValues(std::string const& location) const
{
	static valueMap_t emptymap;
	std::string lowerd = StringToLower(location);
	const std::vector<std::string>& loclist = GetLocationVector(lowerd);
	sectionsMap_t::const_iterator sit = root_section.sections.find(loclist[0]);
	if (sit == root_section.sections.end()) {
		LOG_L(L_WARNING, "Section %s missing in file %s",
				loclist[0].c_str(), filename.c_str());
		return emptymap;
	}
	TdfSection* sectionptr = sit->second;
	std::string searchpath = loclist[0]; // for error-messages
	for (unsigned int i=1; i < loclist.size(); i++) {
		searchpath += '\\';
		searchpath += loclist[i];
		sit = sectionptr->sections.find(loclist[i]);
		if (sit == sectionptr->sections.end()) {
			LOG_L(L_WARNING, "Section %s missing in file %s",
					searchpath.c_str(), filename.c_str());
			return emptymap;
		}
		sectionptr = sit->second;
	}
	return sectionptr->values;
}

std::vector<std::string> TdfParser::GetSectionList(std::string const& location) const
{
	const std::string& lowerd = StringToLower(location);
	const std::vector<std::string>& loclist = GetLocationVector(lowerd);
	const sectionsMap_t* sectionsptr = &root_section.sections;

	std::vector<std::string> returnvec;

	if (!loclist[0].empty()) {
		std::string searchpath;
		for (const auto& loc: loclist) {
			searchpath += loc;
			if (sectionsptr->find(loc) == sectionsptr->end()) {
				LOG_L(L_WARNING, "Section %s missing in file %s",
						searchpath.c_str(), filename.c_str());
				return returnvec;
			}
			sectionsptr = &sectionsptr->find(loc)->second->sections;
			searchpath += '\\';
		}
	}

	for (const auto& s: *sectionsptr) {
		returnvec.push_back(s.first);
		StringToLowerInPlace(returnvec.back());
	}

	return returnvec;
}

bool TdfParser::SectionExist(std::string const& location) const
{
	const std::string& lowerd = StringToLower(location);
	const std::vector<std::string>& loclist = GetLocationVector(lowerd);

	sectionsMap_t::const_iterator sit = root_section.sections.find(loclist[0]);

	if (sit == root_section.sections.end())
		return false;

	TdfSection* sectionptr = sit->second;
	for (unsigned int i = 1; i < loclist.size(); i++) {
		sit = sectionptr->sections.find(loclist[i]);
		if (sit == sectionptr->sections.end()) {
			return false;
		}
		sectionptr = sectionptr->sections[loclist[i]];
	}
	return true;
}

std::vector<std::string> TdfParser::GetLocationVector(std::string const& location) const
{
	const std::string& lowerd = StringToLower(location);

	std::vector<std::string> loclist;
	std::string::size_type start = 0;
	std::string::size_type next = 0;

	while ((next = lowerd.find_first_of('\\', start)) != std::string::npos) {
		loclist.push_back(lowerd.substr(start, next-start));
		start = next + 1;
	}
	loclist.push_back(lowerd.substr(start));

	return loclist;
}

float3 TdfParser::GetFloat3(float3 def, std::string const& location) const
{
	std::string s = SGetValueDef("", location);
	if (s.empty()) {
		return def;
	}
	float3 ret;
	ParseArray(s, &ret.x, 3);
	return ret;
}
