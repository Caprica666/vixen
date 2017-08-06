#include "vixen.h" 
#include "render/texfont.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(TextGeometry, TriMesh, VX_TextGeometry);

static const TCHAR* donames[] =
{	TEXT("SetText"), TEXT("SetTextBox"), TEXT("SetFontName"), TEXT("SetTextSize"), TEXT("SetNormal") };

const TCHAR** TextGeometry::DoNames = donames;

#define	TEXT_MakeGlyphs		2
#define	TEXT_SetImage		4
#define	TEXT_FontLoading	8
#define	TEXT_AppChanged		16

TextGeometry::TextGeometry()
 :	TriMesh(VertexPool::COLORS | VertexPool::TEXCOORDS),
	m_FontName("default.txf"),
	m_Normal(0, 0, 1),
	m_Color(-1, -1, -1, -1),
	m_TextBox(-0.5f, 0.5f, 0, -0.5f, 0.05f, 0)
{
	m_Font = NULL;
	m_FontImage = new Texture;
	m_Status = TEXT_MakeGlyphs | TEXT_SetImage;
	SetActive(false);
}

TextGeometry::~TextGeometry() { m_FontImage = (Texture*) NULL; }

/*!
 * @fn void TextGeometry::SetFontName(const TCHAR* name)
 * @param name font name
 *
 * Sets the name of the font to use for text display.
 * Setting the font does not take effect until
 * the next text display. Fonts are kept in .TXF files
 * and must be in the same directory as the texture files.
 *
 * @see Texture::Load
 */
void TextGeometry::SetFontName(const TCHAR* name)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_TextGeometry, TEXT_SetFontName) << this << name;
   VX_STREAM_END(  )

	if (m_FontName.CompareNoCase(name) == 0)
		return;
	m_FontName = name;
	m_Status |= TEXT_FontLoading;
	m_FontImage->Load(name);
}


/*!
 * @fn Box2 TextGeometry::GetTextExtent(const TCHAR *string) const
 * @param string	null-terminated string to get text extent for
 *
 * Computes the width and height, in pixels, of the text that would
 * result from creating a bitmap from the given string.  You can
 * use this result to determine the size of the TextGeometry needed
 * to accomodate any given string.
 *
 * Text is drawn with its local origin at the baseline of the top left
 * character of the string. The text extent box reflects this. The minimum
 * of pixels above the baseline of the first line of text). The
 * minimum Y is the amount below the baseline of the rest of the text.
 * For a multi-line string, this includes the height of the lines below
 * the first. For a single line of text, negating the minimum Y gives
 * the ascent and the maximum Y is the descent. 
 *
 * @see TextGeometry::SetBound TextGeometry::SetTextSize
 */
Box2 TextGeometry::GetTextExtent(const TCHAR *string) const
{
	int			width, ascent, descent;
	int			type;
	TexFont*	fontdata = (TexFont*) m_FontImage->GetBitmap(&type);
	Core::String	tmp(string);

	if (fontdata == NULL)
		return Box2(0,0,0,0);
	txfGetStringMetrics(fontdata, (TCHAR*) string, STRLEN(string), &width, &ascent, &descent);
	float w = float(width);
	float h = float(descent + ascent);
	return Box2(0, w, -float(descent), float(ascent));
}

/*!
 * @fn void TextGeometry::SetText(const TCHAR* str)
 * @param str	null-terminated string to depict
 *
 * Each character in the input string is mapped to a font character
 * and becomes a rectangle in the text geometry. The appropriate
 * font glyph is texture mapped onto the rectangle to make the character.
 * The current font, text dimensions and text appearance
 * control how the text looks.
 *
 * @see TextGeometry::SetAppIndex TextGeometry::SetFontName
 */
void TextGeometry::SetText(const TCHAR* str)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_TextGeometry, TEXT_SetText) << this << str;
   VX_STREAM_END(  )

	m_Text = str;
	m_Status |= TEXT_MakeGlyphs;
}

/*!
 * @fn void TextGeometry::SetTextBox(const Box3& box)
 * @param box	3D axially-aligned bounding box for text
 *
 * Establishes the rectangle within which text is displayed.
 * The rectangle is in the parent coordinate system.
 * Setting the text box changes the local matrix of the shape
 * to scale and position the text so it fills the rectangle.
 *
 * @see TextGeometry::GetTextExtent TextGeometry::SetTextSize
 */
void TextGeometry::SetTextBox(const Box3& box)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_TextGeometry, TEXT_SetTextBox) << this << box.min << box.max;
   VX_STREAM_END(  )

	m_TextBox = box;
	m_Status |= TEXT_MakeGlyphs;
}

/*!
 * @fn void TextGeometry::SetTextSize(const Vec2& v)
 * @param v	2D vector giving text dimensions in characters
 *
 * Establishes the size of the text box in characters
 * instead of world coordinates.
 *
 * @see TextGeometry::SetTextBox TextGeometry::GetTextExtent
 */
void TextGeometry::SetTextSize(const Vec2& v)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_TextGeometry, TEXT_SetTextSize) << this << v.x << v.y;
   VX_STREAM_END(  )

	m_TextSize = v;
	m_Status |= TEXT_MakeGlyphs;
}

/*!
 * @fn void TextGeometry::SetNormal(const Vec3& v)
 * @param v	face normal for text, should be a coordinate axis
 *
 * Establishes the face normal for the text plane. Normally,
 * this is the Z axis.
 */
void TextGeometry::SetNormal(const Vec3& v)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_TextGeometry, TEXT_SetNormal) << this << v;
   VX_STREAM_END(  )

	m_Normal = v;
	m_Normal.Normalize();
	m_Status |= TEXT_MakeGlyphs;
}

/****
 *
 * class TextModel override for SharedObj::Do
 *	TEXT_SetText		char*
 *	TEXT_SetFontName	char*
 *	TEXT_SetAppIndex	int
 *	TEXT_SetTextBox		Box3
 *	TEXT_SetTextSize	Vec2
 *
 ****/
bool TextGeometry::Do(Messenger& s, int op)
{
	TCHAR	txtbuf[5000];
	Box3	box;
	Vec2	v;
	Vec3	nml;

	switch (op)
	{
		case TEXT_SetFontName:
		s >> txtbuf;
		SetFontName(Core::String(txtbuf));
		break;

		case TEXT_SetText:
		s >> txtbuf;
		SetText(Core::String(txtbuf));
		break;

		case TEXT_SetTextBox:
		s >> box.min >> box.max;
		SetTextBox(box);
		break;

		case TEXT_SetTextSize:
		s >> v.x >> v.y;
		SetTextSize(v);
		break;

		case TEXT_SetNormal:
		s >> nml;
		SetNormal(nml);
		break;

		default:
		return TriMesh::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << TextGeometry::DoNames[op - TEXT_SetText]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * class Sprite override for SharedObj::Copy
 *
 ****/
bool TextGeometry::Copy(const SharedObj* srcobj)
{
	ObjectLock dlock(this);
	ObjectLock slock(srcobj);
	if (!TriMesh::Copy(srcobj))
		return false;
	const TextGeometry* src = (const TextGeometry*) srcobj;
	if (src->IsClass(VX_TextGeometry))
	{
		m_FontName = src->m_FontName;
		m_TextBox = src->m_TextBox;
		m_TextSize = src->m_TextSize;
		m_Status = src->m_Status;
		m_FontImage = src->m_FontImage;
		m_Font = src->m_Font;
	}
	return true;
}

/****
 *
 * class Sprite override for SharedObj::Save
 *
 ****/
int TextGeometry::Save(Messenger& s, int opts) const
{
	int32 h = TriMesh::Save(s, opts);
	if (h <= 0)
		return h;
	s << OP(VX_TextGeometry, TEXT_SetFontName) << h << m_FontName;
	if (m_TextSize.x && m_TextSize.y)
		s << OP(VX_TextGeometry, TEXT_SetTextSize) << h << m_TextSize.x << m_TextSize.y;
	else if (!m_TextBox.IsEmpty())
		s << OP(VX_TextGeometry, TEXT_SetTextBox) << h << m_TextBox.min << m_TextBox.max;
	return h;
}

DebugOut& TextGeometry::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	TriMesh::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "\t<attr name='FontName'>" << m_FontName << "</attr>");
	endl(dbg << "\t<attr name='Text'>" << m_Text << "</attr>");
	TriMesh::Print(dbg, opts & PRINT_Trailer);
	return dbg;
}

bool TextGeometry::GetBound(Box3* box) const
{
	box->min.x = m_TextBox.min.x;
	box->min.y = m_TextBox.min.y;
	box->min.z = 0.0f;
	box->max.x = m_TextBox.max.x;
	box->max.y = m_TextBox.max.y;
	box->max.z = 0.00001f;
	return true;
}

Vec3 TextGeometry::GetSortLoc() const
{
	return Vec3(m_TextBox.min.x, m_TextBox.min.y, 0);
}

intptr TextGeometry::Cull(const Matrix* trans, Scene* scene)
{
	intptr culled = TriMesh::Cull(trans, scene);

	if (!UpdateText())
		return 0;
	return culled;
}

bool TextGeometry::UpdateText()
{
	if (m_Font == NULL)
	{
		int type;
		m_Font = (TexFont*) m_FontImage->GetBitmap(&type);
		if (m_Font == NULL)
		{
			if (!(m_Status & TEXT_FontLoading))
			{
				m_Status |= TEXT_FontLoading;
				m_FontImage->Load(m_FontName);
			}
			return false;
		}
	}
	m_Status &= ~TEXT_FontLoading;
	if (m_Text.IsEmpty())
		return false;
	if (m_Status & TEXT_MakeGlyphs)
	{
		Matrix mtx;
		float d = m_Normal.Dot(Model::ZAXIS);

		MakeGlyphs();
		if (d > 0.99f)
			return true;
		if (d < -0.99f)
			mtx.RotationMatrix(Model::YAXIS, PI);
		else if ((d = m_Normal.Dot(Model::YAXIS)) < -0.99f)
		{
			mtx.RotationMatrix(Model::XAXIS, PI / 2);
			mtx.Rotate(Model::XAXIS, PI);
		}
		else if (d > 0.99f)
			mtx.RotationMatrix(Model::XAXIS, -PI / 2);
		else if ((d = m_Normal.Dot(Model::XAXIS)) < -0.99f)
		{
			mtx.RotationMatrix(Model::YAXIS, -PI / 2);
			mtx.Rotate(Model::XAXIS, -PI / 2);
		}
		else if (d > 0.99f)
		{
			mtx.RotationMatrix(Model::YAXIS, PI / 2);
			mtx.Rotate(Model::XAXIS, PI / 2);
		}
		if (!mtx.IsIdentity())
			*this *= mtx;
	}
	return true;
}

bool TextGeometry::SetTexture(Appearance* appear) const
{
	if (m_Status & TEXT_AppChanged)
	{
		PhongMaterial* mtl = (PhongMaterial*) appear->GetMaterial();
		if (mtl)
		{
			VX_ASSERT(mtl->IsKindOf(CLASS_(PhongMaterial)));
			m_Color = mtl->GetDiffuse();
		}
		else
			m_Color.Set(0,0,0);
	}
	if (m_Status & TEXT_SetImage)
	{
		Sampler* smp = appear->GetSampler(0);
		if (smp == NULL)
		{
			smp = new Sampler(m_FontImage);
			smp->SetName(TEXT("diffuse"));
			smp->Set(Sampler::TEXTUREOP, Sampler::DIFFUSE);
			appear->SetSampler(0, smp);
		}
		else if (smp->GetTexture() != (const Texture*) m_FontImage)
			smp->SetTexture(m_FontImage);
	}
	m_Status &= ~(TEXT_AppChanged | TEXT_SetImage);
	return true;
}


/*!
 * @fn bool TextGeometry::MakeGlyphs()
 * Internal routine to generate the geometry for a text string.
 * Each character of the string selects a single glyph from the
 * text font. The origin of the text glyph coordinate system is
 * at the baseline of the top left character. One unit in this coordinate
 * system is a single pixel in the text font image.
 * Input vectors allow the string geometry to be pre-scaled and translated
 * for placement within a different coordinate system.
 */
bool TextGeometry::MakeGlyphs()
{
	size_t			txtlen = m_Text.GetLength() + 1;
	int				c;
	Box2			textrect;
#ifdef _WIN32
	char*			txtbuf = (char*) alloca(txtlen);
#else
	char			txtbuf[txtlen];
#endif
	const char*		sptr = txtbuf;

	if (m_Font == NULL)
		return false;
#if defined(_WIN32) && defined(_UNICODE)
	m_Text.AsMultiByte(txtbuf, 512);
#else
	STRCPY(txtbuf, m_Text);
#endif
	if (m_TextSize.x && m_TextSize.y)
	{
		textrect = GetTextExtent(TEXT("W"));
		textrect.min.x *= m_TextSize.x;
		textrect.max.x *= m_TextSize.x;
		textrect.min.y *= m_TextSize.y;
		textrect.max.y *= m_TextSize.y;
	}
	else
		textrect = GetTextExtent(m_Text);

	Vec3		scale(m_TextBox.Width() / textrect.Width(), m_TextBox.Height() / textrect.Height(), 1);
	Vec3		trans(m_TextBox.min.x - textrect.min.x * scale.x,
					  m_TextBox.min.y - textrect.min.y * scale.y,
					  m_TextBox.min.z);
	float		left = trans.x;
	float		lineheight = (float) m_Font->max_ascent + m_Font->max_descent;
	VertexIndex	indx[6] = { 0, 1, 2, 2, 3, 0 };
	int			vtxindex = 0;

	trans.y = m_TextBox.max.y - textrect.max.y * scale.y;
	Empty();
	while (c = *sptr++)
		switch (c)
		{
			case '\r':
			break;

			case '\n':
			trans.x = left;
			trans.y -= scale.y * lineheight;
			break;

			default:
			trans.x += AddGlyph(m_Font, c, &scale, &trans);
			for (int i = 0; i < 6; ++i)
				AddIndex(indx[i] + vtxindex);
			vtxindex += 4;
			break;
		}
	m_Status &= ~TEXT_MakeGlyphs;
	SetChanged(false);
	SetActive(true);
	return true;
}

/*!
 * @fn float TextGeometry::AddGlyph(TexFont* txf, int c, Vec3* scale, Vec3* trans)
 * @param txf	font information
 * @param c		character to make glyph for
 * @param scale	scale factor
 * @param trans	translation factor
 *
 * Internal routine computes the vertices and texture coordinates for
 * the triangle fan primitive that describes a glyph rectangle.
 * The vertices are the dimensions of the glyph in pixels but
 * scaled and translated by the input vectors. Y = 0 is at the baseline
 * of the glyph, X = 0 is at the far left.
 * The texture coordinates are used to select the character in the font
 * texture to be used on the glyph. The triangle fan and its vertices
 * are added to this mesh
 */
float TextGeometry::AddGlyph(TexFont* txf, int c, Vec3* scale, Vec3* trans)
{
	float	vtx[4][6];
	Color	textcolor(m_Color);
	TexGlyphVertexInfo*	tgvi;

	tgvi = getTCVI(txf, c);
	if (tgvi == NULL)
		return 0.0f;
	vtx[0][0] = tgvi->v0[0] * scale->x + trans->x;
	vtx[0][1] = tgvi->v0[1] * scale->y + trans->y;
	vtx[0][2] = trans->z;
	vtx[0][3] = *((float*) &textcolor);
	vtx[0][4] = tgvi->t0[0];
	vtx[0][5] = tgvi->t0[1];

	vtx[1][0] = tgvi->v1[0] * scale->x + trans->x;
	vtx[1][1] = tgvi->v1[1] * scale->y + trans->y;
	vtx[1][2] = trans->z;
	vtx[1][3] = *((float*) &textcolor);
	vtx[1][4] = tgvi->t1[0];
	vtx[1][5] = tgvi->t1[1];

	vtx[2][0] = tgvi->v2[0] * scale->x + trans->x;
	vtx[2][1] = tgvi->v2[1] * scale->y + trans->y;
	vtx[2][2] = trans->z;
	vtx[2][3] = *((float*) &textcolor);
	vtx[2][4] = tgvi->t2[0];
	vtx[2][5] = tgvi->t2[1];

	vtx[3][0] = tgvi->v3[0] * scale->x + trans->x;
	vtx[3][1] = tgvi->v3[1] * scale->y + trans->y;
	vtx[3][2] = trans->z;
	vtx[3][3] = *((float*) &textcolor);
	vtx[3][4] = tgvi->t3[0];
	vtx[3][5] = tgvi->t3[1];

	AddVertices(&vtx[0][0], 4);
	return (float) tgvi->advance * scale->x;
}
 } // end Vixen