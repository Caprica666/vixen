#pragma once

namespace Vixen
{
	/*
	 * This defines the format of the light constants passed to the pixel shader
	 * for the default light types.
	 */
	typedef struct GPULight::ShaderConstants LightConstants;

	/*!
	* @class DXLight
	* @brief DX11-specific light properties.
	*
	* Implements the CPUT device specific lighting functions.
	*
	* @also LightList GPULight
	* @ingroup vixen
	*/
	class DXLight : public GPULight
	{
	public:
		DXLight(Renderer& render) : GPULight(render) { }
		virtual void	Update(const Matrix* mtx);
	};
}  // end Vixen
