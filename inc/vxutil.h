/****
 *
 * Main header that defines all the available 
 * user interface / navigation utilities
 *
 ****/
#pragma once

#ifdef _MANAGED
#pragma managed(push, off)
#endif

namespace Vixen
{
	/****
	 *
	 * DO NOT CHANGE THE ORDER OF THESE TYPES!
	 * The Java wrappers use the same ordering and they will break
	 * if these types are rearranged.
	 *
	 ****/
	enum vxUtilID
	{
	//	VX_Navigator = VX_NextClass,
	//	VX_Picker,			// 201
	//	VX_RayPicker,		// 202
	//	VX_FrameStats,		// 203
	//	VX_Cursor3D,		// 204
	//	VX_Flyer,			// 205
		VX_SkyDome,			// 206
		VX_DayDome,			// 207
		VX_StarDome,		// 208
		VX_TerrainCollider, // 209
	//	VX_ArcBall,			// 210
	//	VX_NavRecorder,		// 211

	//	VX_MediaSource,			// 212	in media library
	//	VX_SoundPlayer,			// 213	in media library
		VX_SoundListener = 214,	// 214	in media library
		VX_MidiInput = 215,		// 215
		VX_Terrain = 217,		// 217
		VX_MidiOutput = 218,	// 218
		VX_Speaker = 219,		// 219	in media library
		VX_SpeechTranslator = 220, // 220 in media library

	//	VX_NamePicker,			// 221
		VX_TrackBall = 222,		// 222
	//	VX_FaceAnimPlayer,		// 223
	//	VX_FaceAnimator,		// 224
	//	VX_FaceTracker,			// 225
		VX_NextUtil = 240,
	};

}

#include "util/vxnavigator.h"
#include "util/vxflyer.h"
#include "util/vxframestats.h"
#include "util/vxraypicker.h"
#include "util/vxquadtree.h"
#include "util/vxterrain.h"
#include "util/vxterrcoll.h"
#include "util/vxtrackball.h"

#ifdef _MANAGED
#pragma managed(pop)
#endif