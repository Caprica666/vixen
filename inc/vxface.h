/*!
 * @file vface.h
 * @brief Defines classes for facial animation and tracking.
 *
 * These compile time variables control which multimedia classes
 * are included.
 * @code
 *	VIXEN_MEDIA_CV	enables video capture and playback
 *					(MediaSource using OpenCV library)
 *	VIXEN_MEDIA_IPP	enables video playback
 *					(MediaSource using OpenCV library)
 * @endcode
 *
 * @see vxmidi.h vxmediabase.h vxmediacv.h
 */
#pragma once

namespace Vixen
{
#ifndef FAP_COUNT_MAX
#define FAP_COUNT_MAX	88
#endif
}

#include "vxmedia.h"
#include "face/FaceAnimator.h"
#include "face/FaceTracker.h"
#include "face/FaceAnimPlayerMPL.h"

