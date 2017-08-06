#pragma once

namespace Vixen
{
#define	SCENE_LightsChanged	128	// a light source changed

	/*!
	 * @class GLLight
	 * @brief OpenGL-specific light properties.
	 *
	 * Implements the OpenGL device specific lighting functions
	 * for the scene manager OpenGL port.
	 *
	 * @also LightList GPULight
	 * @ingroup vixen
	 */
	class GLLight : public GPULight
	{
	public:
		GLLight(Renderer&, const Light&);
		virtual GPULight* Clone() const;
		virtual void	Load(bool changed);
		virtual void	Detach();
		virtual	void	Enable() const;
		virtual	void	Disable() const;
	};
}	// end Vixen
