#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(SimpleShape, Shape, VX_SimpleShape);

static const TCHAR* opnames[] =
{	TEXT("SetTexture"), TEXT("SetText"), TEXT("SetForeColor"),
	TEXT("UNUSED"), TEXT("SetTextFont"), TEXT("SetAppIndex") };

const TCHAR** SimpleShape::DoNames = opnames;

SimpleShape::SimpleShape() : Shape()
{
	m_ForeColor.Set(1, 1, 1);
	m_TextBox.Set(0, 0, 0, 0);
	m_AppIndex = -1;
}

/*!
 * @fn Texture* SimpleShape::GetTexture()
 *
 * The image chosen is based on the appearance for this shape.
 * If the shape does not have a valid appearance, NULL is returned.
 *
 * @return image for this shape or NULL if no image
 *
 * @see SimpleShape::SetAppearance Appearance::GetTexture Appearance::GetAt
 */
Texture* SimpleShape::GetTexture()
{
	Appearance*	app = GetAppearance();
	Sampler* smp;

	if (app == NULL)
		return NULL;
	smp = app->GetSampler(0);
	if (smp == NULL)
		return NULL;
	return smp->GetTexture();
}

const Texture* SimpleShape::GetTexture() const
{
	return ((SimpleShape*) this)->GetTexture();
}


/*!
 * @fn int SimpleShape::SetTexture(const Texture* image, int minfilter, int magfilter)
 * @param image		image to use for this shape
 * @param minfilter acceptable values are the same as for APPEAR_MinFilter
 * @param magfilter acceptable values are the same as for APPEAR_MagFilter
 *
 * The new image replaces the existing image in this
 * appearance. If the shape is new and does not have an appearance
 * or geometry, default ones are supplied by this function.
 *
 * @return index of appearance updated or added
 *
 * @see SimpleShape::GetTexture Shape::GetAppearance Appearance::SetTexture
 */
void SimpleShape::SetTexture(const Texture* image, int minfilter, int magfilter)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_SimpleShape, SHAPE_SetImage) << this << image;
	VX_STREAM_END( )

	Geometry*		geo = GetGeometry();
	Appearance*		app = MakeImageAppearance();
	Sampler*		sampler = app->GetSampler(0);

	SetAppearance(app);
	sampler->SetTexture(image);
	sampler->Set(Sampler::MINFILTER, minfilter);
	sampler->Set(Sampler::MAGFILTER, magfilter);
	if (geo == NULL)
		MakeImageGeometry();
}


Appearance* SimpleShape::MakeImageAppearance()
{
	Appearance *app = app = new Appearance(1);
	Sampler* smp = app->GetSampler(0);

	app->Set(Appearance::CULLING, false);
	app->Set(Appearance::LIGHTING, false);
	app->Set(Appearance::TRANSPARENCY, true);
	smp->Set(Sampler::MIPMAP, true);
	smp->Set(Sampler::TEXTUREOP, Sampler::EMISSION);
	smp->Set(Sampler::MINFILTER, Sampler::LINEAR);
	smp->Set(Sampler::MINFILTER, Sampler::LINEAR);
	return app;
}

/****
 *
 * Creates geometry for the shape if there is none.
 * The geometry is two coplanar triangles that make
 * a 1x1 square on which to map a texture
 *
 ****/
Geometry* SimpleShape::MakeImageGeometry()
{
	Geometry* geo = GetGeometry();
	if (geo == NULL)
	{
		TriMesh* mesh = new TriMesh(VertexPool::TEXCOORDS, 4);
		GeoUtil::Rect(mesh, 2, 2);
		SetGeometry(mesh);
		return mesh;
	}
	return geo;
}


/*!
 *
 * @fn void SimpleShape::SetText(const TCHAR* string)
 * @param string	null-terminated string to display as text
 *
 * Creates text geometry or updates the existing text geometry
 * to describe the input string.
 *
 * @see SimpleShape::SetTexture TextGeometry TextGeometry::SetText
 */
void SimpleShape::SetText(const TCHAR* string)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_SimpleShape, SHAPE_SetText) << this << string;
	VX_STREAM_END( )

	TextGeometry* text = (TextGeometry*) GetGeometry();
	if (string)
	{
		if (GetAppearance() == NULL)
			SetAppearance(MakeTextAppearance());
		if (text == NULL)
			text = MakeTextGeometry();
		text->SetText(string);
	}
}

Appearance* SimpleShape::MakeTextAppearance()
{
	Appearance *app = new Appearance(m_ForeColor);
	Sampler* sampler = new Sampler();
	
	app->Set(Appearance::CULLING, false);
	app->Set(Appearance::LIGHTING, false);
	sampler->SetName(TEXT("diffuse"));
	sampler->Set(Sampler::TEXTUREOP, Sampler::DIFFUSE);
	sampler->Set(Sampler::MINFILTER, Sampler::LINEAR);
	sampler->Set(Sampler::MAGFILTER, Sampler::LINEAR);
	sampler->Set(Sampler::MIPMAP, true);
	app->SetSampler(0, sampler);
	return app;
}

/*!
 * @fn void SimpleShape::SetForeColor(const Col4 &col)
 * @param col	color to use for displaying text
 *
 * The foreground color is the diffuse color of the material
 * for this shape's appearance. It is used to display
 * text as well as any meshes which use the appearance.
 *
 * @see SimpleShape:SetAppIndex Shape::GetAppearances Appearance::SetMaterial
 */
void SimpleShape::SetForeColor(const Col4 &col)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_SimpleShape, SHAPE_SetForeColor) << this << col.r << col.g << col.b;
	VX_STREAM_END(  )

	m_ForeColor = col;
	Appearance* a = GetAppearance();
	if (a == NULL)
		a =	MakeTextAppearance();
	if (a == NULL)
		return;
	PhongMaterial* mat = (PhongMaterial*) a->GetMaterial();
	if (mat)
	{
		VX_ASSERT(mat->IsKindOf(CLASS_(PhongMaterial)));
		mat->SetDiffuse(col);
	}
	else
	{
		mat = new PhongMaterial(col);
		a->SetMaterial(mat);
	}
}

TextGeometry *SimpleShape::GetTextGeometry() const
{
	TextGeometry *text = (TextGeometry *) GetGeometry();
	if (text && text->IsClass(VX_TextGeometry))
		return text;
	return NULL;
}

/*!
 * @fn void SimpleShape::SetTextFont(const char *filename)
 * @param fontfile	string containing name of font (including .txf)
 *
 * The font name will be used by text geometry attached to this shape.
 *
 * @see TextGeometry::SetFontName SimpleShape::SetText
 */
void SimpleShape::SetTextFont(const TCHAR *filename)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_SimpleShape, SHAPE_SetTextFont) << this << filename;
	VX_STREAM_END(  )

	TextGeometry *text = GetTextGeometry();
	if (text)
		text->SetFontName(filename);
}

/*!
 * @fn void SimpleShape::SetBound(const Box3* box)
 *
 * The default text box is the box (0,1 0,1).
 *
 * @see Model::SetBound
 */
void SimpleShape::SetBound(const Box3* box)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_SimpleShape, MOD_SetBoxBound) << this << box->min << box->max;
	VX_STREAM_END(  )

	m_TextBox = *box;
}

void SimpleShape::Render(Scene* scene)
{
	TextGeometry* textgeo = (TextGeometry*) GetGeometry();

	if (textgeo && textgeo->IsClass(VX_TextGeometry))
		textgeo->SetTexture(GetAppearance());
}

void SimpleShape::SetBound(const Sphere* sp)
{	Shape::SetBound(sp); }


bool SimpleShape::Do(Messenger& s, int op)
{
	TCHAR		buf[4000];
	int32		n;
	Col4		c;
	SharedObj*		obj;
	const Texture* image;

	switch (op)
	{
		case SHAPE_SetText:
		s >> buf;
		SetText(buf);
		break;

		case SHAPE_SetTextFont:
		s >> buf;
		SetTextFont(buf);
		break;

		case SHAPE_SetForeColor:
		s >> c.r >> c.g >> c.b;
		SetForeColor(c);
		break;

		case SHAPE_SetImage:
		s >> obj >> n;
		if (obj) { VX_ASSERT(obj->IsClass(VX_Image)); }
		image = (const Texture*) obj;
		SetTexture(image, n);
		break; 

		default:
		return Shape::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
	{
		vixen_debug << ClassName() << "::"
				  << SimpleShape::DoNames[op - SHAPE_SetImage]
				   << " " << this;
		endl(vixen_debug);
	}
#endif
	return true;
}

TextGeometry* SimpleShape::MakeTextGeometry()
{
	TextGeometry* text = new TextGeometry;
	text->SetTextBox(m_TextBox);
	SetGeometry(text);
	return text;
}

int SimpleShape::Save(Messenger& s, int opts) const
{
	int32 h = Shape::Save(s, opts);
	if (h <= 0)
		return h;
	if (!m_TextBox.IsEmpty())
		s << OP(VX_SimpleShape, MOD_SetBoxBound) << h << m_TextBox.min << m_TextBox.max;
	return h;
}

bool SimpleShape::Copy(const SharedObj* src_obj)
{
	const SimpleShape*	src = (const SimpleShape*) src_obj;

	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Shape::Copy(src_obj))
	  return false;
	if (src_obj->IsClass(VX_SimpleShape))
	{
		m_TextBox = src->m_TextBox;
		m_ForeColor = src->m_ForeColor;
	}
	return true;
}

DebugOut& SimpleShape::Print(DebugOut& dbg, int opts) const
{
	TextGeometry* text = GetTextGeometry();

	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Shape::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "<attr name='ForeColor'>" << m_ForeColor << "</attr>");
	if (text)
	{
		const TCHAR* s = text->GetFontName();
		if (s)
			endl(dbg << "<attr name='FontName'>" << s << "</attr>");
		s = text->GetText();
		if (s)
			endl(dbg << "<attr name='Text'>" << s << "</attr>");
	}
	return Shape::Print(dbg, opts & PRINT_Trailer);
}

}	// end Vixen