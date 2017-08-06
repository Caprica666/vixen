/*!
 * @file vxmedia.h
 * @brief Defines multimedia classes for speech, sound and video.
 *
 * These compile time variables control which multimedia classes
 * are included.
 * @code
 *	VIXEN_SOUND		enables sound support  (SoundPlayer class)
 *	VIXEN_MEDIA_CV	enables video capture and playback
 *					(MediaSource using OpenCV library)
 *	VIXEN_MEDIA_IPP	enables video playback
 *					(MediaSource using OpenCV library)
 * @endcode
 *
 * @see vxmidi.h vxmediabase.h vxmediacv.h vxsoundplayfm.h
 */
#pragma managed(push, off)
#include "media/VXSoundPlayFM.h"

#ifdef VIXEN_MEDIA_CV
	#include "media/VXMediaBase.h"
	#include "media/VXMediaCV.h"
#elif defined(VIXEN_MEDIA_IPP)
	#include "media/VXMediaBase.h"
	#include "media/VXMediaIPP.h"
#elif defined(VIXEN_MEDIA_DX)
	#include "media/VXMediaBase.h"
	#include "media/VXMediaDX.h"
#endif

#pragma managed(pop)