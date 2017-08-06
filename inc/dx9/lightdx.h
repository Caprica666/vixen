#pragma once

namespace Vixen {
/*!
 * @class DXLightProp
 * @brief DirectX-specific light properties.
 *
 * Implements the DirectX device specific lighting functions
 * for the scene manager DirectX port.
 *
 * @also LightList GPULight
 * @ingroup vixenint
 * @internal
 */
class DXLightProp : public GPULight
{
public:
	DXLightProp(Renderer&, const Light&);
	GPULight& operator=(const GPULight&);
	virtual void	Load(bool changed);
	virtual void	Enable() const;
	virtual void	Disable() const;
	virtual void	Update(const Matrix* mtx);
	virtual void	Detach();

protected:
	D3DLIGHT9	DXLight;
};

} // end Vixen