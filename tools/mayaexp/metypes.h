#pragma once

#include <maya/MTypes.h>
#undef strcasecmp

#include "vixen.h"

/*
 * Maya includes
 */

#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MDagPath.h>
#include <maya/MObject.h>

#define ME_SURFACE	1
#define	ME_MAX_TEXTURES	8
#define	ME_MAX_STATES	16
#define	ME_MAX_SHADERS	32
#define ME_MAX_NAME	256
#define ME_MAX_VTX_SIZE	3 + 3 + 2 * ME_MAX_TEXTURES

#include "medebug.h"

class vixExporter;
extern vixExporter* Exporter;

