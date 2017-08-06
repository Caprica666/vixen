/*!
 * @file vxtextgeom.h
 * @brief Geometry that represents text characters.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxmesh.h vxsimpleshape.h
 */
#pragma once

#include "render/texfont.h"

namespace Vixen {

/*!
 * @class TextGeometry
 * @brief Generates geometry that describes a text string.
 *
 * Each character is represented by a rectangle which uses texture
 * coordinates to select the appropriate symbol from a single font texture
 * containing all the characters.
 *
 * You can add text geometry to a shape just as you would a normal mesh.
 * The appearance associated with the text can be used to control
 * mip-mapping, blending and filtering for the font texture.
 * The material of this appearance controls the text color, which
 * is embedded in the geometry as vertex colors.
 *
 * You can choose from one of several font files provided. You can specify
 * a font file name anywhere a texture file name is allowed. Files ending
 * with ".TXF" are assumed to be font files.
 *
 * The size of the text geometry is specified as a 3D bounding box in the
 * local coordinate system of the shape referencing the geometry.
 * The text will be scaled to fit this rectangle and positioned in Z
 * at the minimum Z value of the requested bounding box. Text has no depth.
 *
 * @see Shape Texture Appearance TriMesh
 */
class TextGeometry : public TriMesh
{
public:
	VX_DECLARE_CLASS(TextGeometry);
	TextGeometry();
	~TextGeometry();

//! Set the text string to convert to geometry.
	void			SetText(const TCHAR* string);
//! Set the name of the font to use.
	void			SetFontName(const TCHAR* name);
//! Get the name of the font.
	const TCHAR*	GetFontName() const		{ return m_FontName; }
//! Get the text string represented by this geometry.
	const TCHAR*	GetText() const			{ return m_Text; }
//! Get the size of the text in characters
	const Vec2&	GetTextSize() const			{ return m_TextSize; }
//! Get the face normal for the text plane.
	const Vec3&	GetNormal() const			{ return m_Normal; }
//! Set text dimensions in terms of world coordinates.
	void			SetTextBox(const Box3&);
//! Set text dimensions in terms of characters.
	void			SetTextSize(const Vec2&);
//! Set the face normal for the text plane.
	void			SetNormal(const Vec3&);
//! Get the pixel size of the text string
	Box2			GetTextExtent(const TCHAR *str) const;

//	Internal functions
	virtual intptr	Cull(const Matrix*, Scene*);
	virtual bool	GetBound(Box3*) const;
	virtual Vec3	GetSortLoc() const;
	virtual bool	Copy(const SharedObj*);
	virtual bool	Do(Messenger& s, int op);
	virtual int		Save(Messenger&, int) const;
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;
	bool			SetTexture(Appearance*) const;

protected:
	bool			MakeGlyphs();
	bool			UpdateText();
	float			AddGlyph(TexFont* txf, int c, Vec3* scale, Vec3* trans);

	/*
	 * TextGeometry::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		TEXT_SetText = TriMesh::TRIMESH_NextOp,
		TEXT_SetTextBox,
		TEXT_SetFontName,
		TEXT_SetTextSize,
		TEXT_SetNormal,
		TEXT_NextOp = TriMesh::TRIMESH_NextOp + 20
	};

protected:
//	Data
	Core::String	m_FontName;
	Core::String	m_Text;
	Ref<Texture>	m_FontImage;
	Box3			m_TextBox;
	Vec2			m_TextSize;
	mutable Col4	m_Color;
	Vec3			m_Normal;
	mutable int		m_Status;
	TexFont*		m_Font;
};

} // end Vixen