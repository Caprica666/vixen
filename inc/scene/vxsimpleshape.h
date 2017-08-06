
/*!
 * @file vxsimpleshape.h
 * @brief Simple shape with a single mesh.
 *
 * Simple shapes usually have a single mesh in which all primitives
 * use the same appearance. They are useful for billboards and sprites.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxbb.h vxshape.h vxsprite.h
 */

#pragma once

namespace Vixen {

class TextGeometry;

/*!
 * @class SimpleShape
 * @brief SimpleShape merges geometry with text or an image.
 *
 * You can seasily et the color or font name for text
 * or add an image to a billboard or sprite.
 * This class makes it easy to update either the geometry or the appearance.
 *
 * @see Shape Appearance TextGeometry BillBoard Sprite
 */
class SimpleShape : public Shape
{
public:
	VX_DECLARE_CLASS(SimpleShape);
	SimpleShape();

//! Sets the image associated with this shape.
	virtual void	SetTexture(const Texture* image,	int minfilter = Sampler::LINEAR, int magfilter = Sampler::LINEAR);
//! Gets the image associated with this shape, if any.
	Texture*		GetTexture();
	const Texture*	GetTexture() const;
//! Associates the given text with this shape.
	virtual void	SetText(const TCHAR* string = NULL);
//! Sets the foreground color.
	void			SetForeColor(const Col4& col);
//! Sets the name of the font to use for displaying text.
	void			SetTextFont(const TCHAR *filename);
//! Establishes the bounding box for text geometry.
	void			SetBound(const Box3*);

	virtual void	Render(Scene* scene);
	virtual void	SetBound(const Sphere*);
	virtual bool	Do(Messenger& s, int op);
	virtual int		Save(Messenger&, int) const;
	virtual bool	Copy(const SharedObj*);
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

protected:
	virtual Appearance*	MakeImageAppearance();
	virtual Appearance*	MakeTextAppearance();
	virtual Geometry*		MakeImageGeometry();
	virtual TextGeometry *MakeTextGeometry();
	TextGeometry *GetTextGeometry() const;

//	Data
	Col4		m_ForeColor;
	Box3		m_TextBox;
	int32		m_AppIndex;
};

} // end Vixen