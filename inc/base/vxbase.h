/*!
 * @file vxbase.h
 * @brief Contains base types for Visual Experience Engine which are not graphics-related.
 *
 * These classes support serializable objects, event management and infrastructure
 * for loading of files from the web.
 *
 * @see vcore
 * @author Nola Donato
 *
 */
#pragma once

#include "vcore/vcore.h"

namespace Vixen
{
	typedef	intptr	Window;
};

#include "base/vxcol.h"
#include "base/vxcol.inl"
#include "base/vxobj.h"
#include "base/vxarray.h"
#include "base/vxdict.h"
#include "base/vxgroup.h"
#include "base/vxmess.h"
#include "base/vxweakref.h"
#include "base/vxarray.inl"
#include "base/vxevent.h"
#include "base/vxevent.inl"
#include "base/vxobj.inl"
#include "base/vxmess.inl"
#include "base/vxweakref.inl"
#include "base/vxfile.h"
#include "base/vxload.h"
#include "base/vxworld.h"
#include "base/vxbitmap.h"
#include "base/vxmath.h"
#include "base/vxmatrix.h"
#include "base/vxmath.inl"
#include "base/vxmatrix.inl"
#include "base/vxbox.h"
#include "base/vxbox.inl"
#include "base/vxsphere.h"
