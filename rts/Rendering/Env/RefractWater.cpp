/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#include "RefractWater.h"
#if 0
#include "WaterRendering.h"
#include "Rendering/GlobalRendering.h"
#include "Rendering/GL/myGL.h"
#include "Map/MapInfo.h"
#include "Map/ReadMap.h"
#include "System/bitops.h"

CRefractWater::CRefractWater()
	: CAdvWater(false)
	, subSurfaceTex(0)
{
	LoadGfx();
}

void CRefractWater::LoadGfx()
{
	glGenTextures(1, &subSurfaceTex);
	glBindTexture(GL_TEXTURE_RECTANGLE, subSurfaceTex);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, 3, globalRendering->viewSizeX, globalRendering->viewSizeY, 0, GL_RGB, GL_INT, 0);
	waterFP = LoadFragmentProgram("ARB/waterRefractTR.fp");
}

CRefractWater::~CRefractWater()
{
	if (subSurfaceTex != 0) {
		glDeleteTextures(1, &subSurfaceTex);
	}
}

void CRefractWater::Draw()
{
	if (!waterRendering->forceRendering && !readMap->HasVisibleWater())
		return;

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, subSurfaceTex);
	glEnable(GL_TEXTURE_RECTANGLE);
	glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, globalRendering->viewPosX, 0, globalRendering->viewSizeX, globalRendering->viewSizeY);

	SetupWaterDepthTex();

	glActiveTexture(GL_TEXTURE0);

	// GL_TEXTURE_RECTANGLE uses texcoord range 0 to width, whereas GL_TEXTURE_2D uses 0 to 1
	const float v[] = {10.0f * globalRendering->viewSizeX, 10.0f * globalRendering->viewSizeY, 0.0f, 0.0f};
	glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 2, v);

	CAdvWater::Draw(false);

	glActiveTexture(GL_TEXTURE3);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glActiveTexture(GL_TEXTURE2);
	glDisable(GL_TEXTURE_RECTANGLE);
	glActiveTexture(GL_TEXTURE0);
}

void CRefractWater::SetupWaterDepthTex()
{
	glActiveTexture(GL_TEXTURE3);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, readMap->GetShadingTexture()); // the shading texture has water depth encoded in alpha
	glEnable(GL_TEXTURE_GEN_S);

	const float splane[] = {1.0f / (mapDims.pwr2mapx * SQUARE_SIZE), 0.0f, 0.0f, 0.0f};
	const float tplane[] = {0.0f, 0.0f, 1.0f / (mapDims.pwr2mapy * SQUARE_SIZE), 0.0f};

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, splane);

	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, tplane);
}

#else

CRefractWater::CRefractWater() = default;
CRefractWater::~CRefractWater() = default;

void CRefractWater::Draw() {}
#endif

