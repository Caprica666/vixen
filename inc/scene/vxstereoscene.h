

/*!
 * @file vxdualscene.h
 * @brief Dual processor 3D scene display.
 *
 * Distributes display traversal and rendering across two threads
 * to leverage dual processor machines.
 *
 * @author Nola Donato
 * @ingroup vixen
 * @internal
 *
 * @see vxscene.h
 */
#pragma once

namespace Vixen {

#ifndef VX_NOTHREAD

/*!
 * @class StereoScene
 * @brief Renders a scene as stereo pairs.
 *
 * The simulation and display traversal is done once but
 * the rendering is done twice - for the left and right eyes.
 * The camera used for each eye is slightly different.
 * This class sets the cull volume of the camera to include
 * everything visible with both eyes.
 *
 * @see Scene DualScene Camera::SetEyeSep Camera::SetCullVol
 * @ingroup vixen
 */
class StereoScene : public Scene
{
public:
	StereoScene(Renderer* r = NULL) : Scene(r) { };
	StereoScene(Scene& src) : Scene(src) { };

	virtual void	DoFrame();
};

#endif	// VX_NOTHREAD

} // end Vixen
