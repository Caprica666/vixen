#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Sprite, SimpleShape, VX_Sprite);

static const TCHAR* opnames[] =
{	TEXT("SetTexture"), TEXT("SetUpperLeft"), TEXT("SetLowerRight"), TEXT("SetText") };

const TCHAR** Sprite::DoNames = opnames;

Sprite::Sprite()
 :	SimpleShape(),
	mUpperLeft(0.0, 0.0),
	mLowerRight(1.0, 1.0),
	mLastViewVolume(-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f)
{
	SetChanged(true);
	SetCulling(false);
	SetHints(MORPH);
}

bool Sprite::CalcMatrix(Matrix* trans, Scene* scene) const
{
	Camera*	camera = scene->GetCamera();
	Vec3	pos;

	trans->GetTranslation(pos);
	pos.z -= 4.0f * camera->GetHither();
	trans->SetTranslation(pos);
	return true;
}

void Sprite::Render(Scene* scene)
{
	Camera*			camera = scene->GetCamera();
	Box3			vv = camera->GetViewVol();
	bool			hastext = false;
	TextGeometry*	text;

	if ((text = (TextGeometry*) GetGeometry()) &&
		(hastext = text->IsClass(VX_TextGeometry)))
	{
		if (HasChanged() || text->HasChanged())
		{
			Box3 textbox;
			float w = vv.Width();
			float h = vv.Height();
			float tx = vv.min.x;
			float ty = vv.max.y;

			textbox.min.Set(tx + (mUpperLeft.x - 0.5f) * w, ty - (mLowerRight.y - 0.5f) * h, 0);
			textbox.max.Set(tx + (mLowerRight.x - 0.5f) * w, ty - (mUpperLeft.y - 0.5f) * h, 0);
			text->SetTextBox(textbox);
			text->SetChanged(false);
			SetChanged(false);
		}
	}
	else if (HasChanged())
	{
		if (hastext)
			MakeTextGeometry();
		else
			MakeImageGeometry();
	}
/*
 * Sprite geometry locations are relative to the camera view volume.
 * If the view volume has been updated since last call, we need
 * to update the geometry to reflect the change
 */
	else
		UpdateGeometry(scene);
	SimpleShape::Render(scene);
}

void Sprite::UpdateGeometry(Scene* scene)
{
	Camera*	camera = scene->GetCamera();
	Box3		vv = camera->GetViewVol();
	Geometry*	geo = GetGeometry();

	if (vv == mLastViewVolume)
		return;
	if ((geo == NULL) || !geo->IsClass(VX_Mesh))
		return;

	VertexArray* verts = ((Mesh*) geo)->GetVertices();
	if (!verts)
		return;
	VertexPool::Iter iter(verts);
	while (iter.Next())
	{
		Vec3* loc = iter.GetLoc();
		
		loc->x = loc->x * vv.min.x /  mLastViewVolume.min.x;
		loc->y = loc->y * vv.min.y /  mLastViewVolume.min.y;
	}
	mLastViewVolume = vv;
}

/*!
 * @fn void Sprite::SetUpperLeft(const Vec2& ul)
 *
 * Sets the position of the upper left corner of the sprite.
 * Sprite positions are relative to the camera view volume
 * with the (1, 1) referencing the upper left corner of the
 * visible screen area.
 *
 * @see Sprite::SetLowerRight Sprite::MakeImageGeometry
 */
void Sprite::SetUpperLeft(const Vec2& ul)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_Sprite, SPRITE_SetUpperLeft) << this << ul.x << ul.y;
   VX_STREAM_END(  )

	mUpperLeft = ul;
	m_TextBox.min.x = ul.x - 0.5f;
	m_TextBox.max.y = ul.y - 0.5f;
}

/*!
 * @fn void Sprite::SetLowerRight(const Vec2& lr)
 *
 * Sets the position of the lower right corner of the sprite.
 * Sprite positions are relative to the camera view volume
 * with the (1, 1) referencing the lower right corner of the
 * visible screen area.
 *
 * @see Sprite::SetUpperLeft Sprite::MakeImageGeometry
 */
void Sprite::SetLowerRight(const Vec2& lr)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_Sprite, SPRITE_SetLowerRight) << this << lr.x << lr.y;
   VX_STREAM_END(  )

	mLowerRight = lr;
	m_TextBox.max.x = lr.x - 0.5f;
	m_TextBox.min.y = lr.y - 0.5f;
}

/*!
 * @fn void Sprite::MakeImageGeometry()
 *
 * Constructs the sprite geometry based on the upper left and
 * lower right sprite positions. Geometry and appearance tables
 * for the sprite shape will not be constructed if they already exist.
 *
 * The default sprite geometry is two triangles with locations and
 * texture coordinates. The triangles will use the appearance index
 * set by SimpleShape::SetAppIndex.
 *
 * The default sprite appearance is non-lit but textured with transparency
 * and mip-mapping enabled. By default, a texture image is not supplied.
 *
 * @see Sprite::SetUpperLeft Sprite::SetLowerRight 
 */
Geometry* Sprite::MakeImageGeometry()
{
	Vec2	ul(mUpperLeft);
	Vec2	lr(mLowerRight);
	Scene* scene = GetMainScene();
	if (scene)
	{
		Camera*	cam = scene->GetCamera();
		Box3	vv;
		float	w, h;

		cam->GetViewVol(&vv);
		mLastViewVolume = vv;
		w = vv.Width();
		h = vv.Height();
		ul = Vec2(vv.min.x + (mUpperLeft.x - 0.5f) * w, vv.max.y - (mUpperLeft.y - 0.5f) * h);
		lr = Vec2(vv.min.x + (mLowerRight.x - 0.5f) * w, vv.max.y - (mLowerRight.y - 0.5f) * h);
	}
	TriMesh* mesh = (TriMesh*) GetGeometry();
	if (mesh && mesh->IsClass(VX_Mesh) && (mesh->GetNumVtx() == 4))
	{
		VertexPool::Iter iter(((Mesh*) mesh)->GetVertices());
		*iter.GetLoc(0) = Vec3(ul.x, ul.y, 0.0f);
		*iter.GetLoc(1) = Vec3(lr.x, ul.y, 0.0f);
		*iter.GetLoc(2) = Vec3(lr.x, lr.y, 0.0f);
		*iter.GetLoc(3) = Vec3(ul.x, lr.y, 0.0f);
	}
	else
	{
		// Create a rectangular mesh.
		VertexPool::Iter iter(((Mesh*) mesh)->GetVertices());
		float* vptr = iter.GetVtx(0);
		*((Vec3*) vptr) = Vec3(ul.x, ul.y, 0.0f);
		*((Vec2*) (vptr + 3)) = Vec2(0.0f, 0.0f);
		vptr = iter.GetVtx(1);
		*((Vec3*) vptr) = Vec3(lr.x, ul.y, 0.0f);
		*((Vec2*) (vptr + 3)) = Vec2(1.0f, 0.0f);
		vptr = iter.GetVtx(2);
		*((Vec3*) vptr) = Vec3(lr.x, lr.y, 0.0f);
		*((Vec2*) (vptr + 3)) = Vec2(1.0f, 1.0f);
		vptr = iter.GetVtx(3);
		*((Vec3*) vptr) = Vec3(ul.x, lr.y, 0.0f);
		*((Vec2*) (vptr + 3)) = Vec2(0.0f, 1.0f);

		// Create the primatives for the rect.
		int32 indices[6] = {0, 1, 2, 2, 3, 0};
		mesh->AddIndices(indices, 6);
		SetGeometry(mesh);
	}
	SetChanged(false);
	return mesh;
}

/****
 *
 * class Sprite override for SharedObj::Copy
 *
 ****/
bool Sprite::Copy(const SharedObj* srcobj)
{
	ObjectLock dlock(this);
	ObjectLock slock(srcobj);
	if (!SimpleShape::Copy(srcobj))
		return false;
	const Sprite* src = (const Sprite*) srcobj;
	if (src->IsClass(VX_Sprite))
	{
		mUpperLeft = src->mUpperLeft;
		mLowerRight = src->mLowerRight;
		SetChanged(true);
	}
	return true;
}

/****
 *
 * class Sprite override for SharedObj::Save
 *
 ****/
int Sprite::Save(Messenger& s, int opts) const
{
	int32 h = SimpleShape::Save(s, opts);
	if (h <= 0)
		return h;
	s << OP(VX_Sprite, SPRITE_SetUpperLeft) << h << mUpperLeft.x << mUpperLeft.y;
	s << OP(VX_Sprite, SPRITE_SetLowerRight) << h << mLowerRight.x << mLowerRight.y;
	return h;
}

DebugOut& Sprite::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	SimpleShape::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "\t<attr name='UpperLeft'>" << mUpperLeft << "</attr>");
	endl(dbg << "\t<attr name='LowerRight'>" << mLowerRight << "</attr>");
	return SimpleShape::Print(dbg, opts & PRINT_Trailer);
}

/****
 *
 * class Sprite override for SharedObj::Do
 *	SPRITE_SetImage			Texture*
 *	SPRITE_SetUpperLeft		<Vec2>
 *	SPRITE_SetLowerRight	<Vec2>
 *	SPRITE_MakeImages		char* filename
 *
 ****/
bool Sprite::Do(Messenger& s, int op)
{
	Vec2	v;
	SharedObj*		image;
	int32		n;

	switch (op)
	{
		case SPRITE_SetImage:
		s >> image >> n;
		VX_ASSERT(image->IsClass(VX_Image));
		SetTexture((Texture*) image, n);
		break;

		case SPRITE_SetUpperLeft:
		s >> v.x >> v.y;
		SetUpperLeft(v);
		break;

		case SPRITE_SetLowerRight:
		s >> v.x >> v.y;
		SetLowerRight(v);
		break;

		default:
		return SimpleShape::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Sprite::DoNames[op - SPRITE_SetImage]
					   << " " << this);
#endif
	return true;
}

}	// end Vixen