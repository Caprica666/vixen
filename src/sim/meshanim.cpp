#include "vixen.h"

#ifndef VX_NOTHREAD
#include "computethread.h"
#endif

namespace Vixen {

VX_IMPLEMENT_CLASSID(MeshAnimator, Deformer, VX_MeshAnimator);

MeshAnimator::MeshAnimator()
  :	Deformer(),
	m_TimeInc(0.0f),
	m_CurIndex(-1)
{
	SetControl(CONTROL_CHILDREN);
}

/*!
 * @fn void	MeshAnimator::SetTimeInc(float t)
 * @param t			time increment in seconds
 *
 * Establishes the amount of time between sources for animation.
 * To select the target mesh vertices, the elapsed time is used
 * to index into the sources array. If a non-positive value is
 * selected, the target mesh is not updated.
 *
 * @see MeshAnimator::SetSource
 */
void	MeshAnimator::SetTimeInc(float t)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_MeshAnimator, MESHANIM_SetTimeInc) << this << t;
	VX_STREAM_END(  )

	m_TimeInc = t;
}

/*!
 * @fn void MeshAnimator::SetSource(int i, VertexArray* verts)
 * @param i		0-based index of source
 * @param verts array of vertices to use as input to deformation
 *
 * The input array references a set of vertices with the same ordering as
 * the target mesh. This source vertex array is multiplied by it's blend weight
 * to contribute to the computation of the target mesh vertices.
 * Setting a source to NULL effectively removes its contribution to the
 * target mesh. Setting the blend weight to 0 also does this.
 *
 * @see MeshAnimator::SetWeight
 */
void MeshAnimator::SetSource(int i, VertexArray* verts)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_MeshAnimator, MESHANIM_SetSource) << this << i << verts;
	VX_STREAM_END(  )

	int n;

	if (i < 0)
		m_Sources.Append(verts);
	else
		m_Sources.SetAt(i, verts);
	n = (int) m_Sources.GetSize() - 1;
	if (n > 0)
		SetDuration(n * m_TimeInc, ONLYME);
}

void MeshAnimator::Clear()
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_MeshAnimator, MESHANIM_Clear) << this;
	VX_STREAM_END(  )

	m_Sources.Empty();
	if (GetControl() & CONTROL_CHILDREN)
	{
		GroupIterNotSafe<Engine> iter(this, CHILDREN);
		Engine* e;

		while (e = iter.Next())
			if (e->IsClass(VX_MeshAnimator))
				((MeshAnimator*) e)->Clear();
	}
};

int MeshAnimator::Save(Messenger& s, int opts) const
{
	int32 h = Deformer::Save(s, opts);
	int n;

	if (h < 0)
		return h;
	if (h > 0)
		s << OP(VX_MeshAnimator, MESHANIM_SetTimeInc) << h << m_TimeInc;
	n = (int) m_Sources.GetSize();
	for (int i = 0; i < n; ++i)
	{
		const SharedObj* obj = m_Sources.GetAt(i);
		if (obj && (obj->Save(s, opts) >= 0) && h)
			s << OP(VX_MeshAnimator, MESHANIM_SetSource) << h << i << obj;
	}
	return h;
}

bool MeshAnimator::Do(Messenger& s, int op)
{
	Matrix			mtx;
	float			v;
	int				i;
	SharedObj*		obj;

	switch (op)
	{
		case MESHANIM_SetSource:
		s >> i;
		s >> obj;
		SetSource(i, (VertexArray*) obj);
		break;

		case MESHANIM_SetTimeInc:
		s >> v;
		SetTimeInc(v);
		break;


		default:
		return Engine::Do(s, op);
	}
	return true;
}

DebugOut& MeshAnimator::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Engine::Print(dbg, opts & ~PRINT_Trailer);
	if (opts & PRINT_Data)
	{
		endl(dbg << "<attr name='TimeInc'>" << m_TimeInc << "</attr>");
		endl(dbg << "<attr name='Sources'>");
		m_Sources.Print(dbg, opts & ~PRINT_Trailer);
		endl(dbg << "</attr>");
	}
	return Engine::Print(dbg, opts & PRINT_Trailer);
}


bool MeshAnimator::RecordTarget()
{
	const VertexArray*	srcverts = m_TargetVerts;
	VertexArray*		dstverts;
	int					numsources = (int) m_Sources.GetSize();

	if (srcverts == NULL)
		return false;
	dstverts = (VertexArray*) srcverts->Clone();
	if (dstverts)
		SetSource(numsources, dstverts);
	return true;
}

bool MeshAnimator::Copy(const SharedObj* src_obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Engine::Copy(src_obj))
		return false;
	const MeshAnimator* src = (const MeshAnimator*) src_obj;
	if (src->IsClass(VX_MeshAnimator))
    {
		m_TimeInc = src->m_TimeInc;
		m_Sources.Copy(&(src->m_Sources));
   }
    return true;
}

bool MeshAnimator::OnReset()
{
	m_CurIndex = -1;
	return true;
}

Morph* MeshAnimator::FindMorph()
{
	Engine* e = First();

	// Find the first Morph child
	while (e)
		if (e->IsClass(VX_Morph))
			return (Morph*) e;
		else
			e = e->Next();
	return NULL;
}


bool MeshAnimator::Init(SharedObj* target, VertexCache* vdict)
{
	if (GetControl() & (BLEND_BETWEEN | BLEND_TO))
	{
		Morph* morph = FindMorph();

		if (morph == NULL)
		{
			Core::String name(GetName());
			morph = new Morph();
			morph->SetName(name + ".morph");
		}
		if (morph->GetTarget() == NULL)
		{
			SetTarget(NULL);
			morph->SetRestLocs(GetRestLocs());
			morph->SetRestNormals(GetRestNormals());
			morph->SetTarget(GetTarget());
			morph->Init(target, vdict);
			Deformer::Init(target, vdict);
		}
		if (morph->GetSource(0) == NULL)
		{
			const VertexArray* rest = GetRestLocs();
			VertexArray* v = new VertexArray(rest->GetLayout()->Descriptor, rest->GetNumVtx());
			v->SetNumVtx(rest->GetNumVtx());
			morph->SetSource(0, (VertexArray*) v);
			morph->SetSource(1, (VertexArray*) v->Clone());
		}
		return true;
	}
	return Deformer::Init(target, vdict);
}

bool MeshAnimator::Eval(float t)
{
	int n = (int) m_Sources.GetSize();
	int c = GetControl();
	bool changed = true;

	if (c & RECORD)
	{
		m_CurIndex = n;
		RecordTarget();
		return true;
	}
	if ((m_TimeInc <= 0) || (n == 0))
	{
		m_CurIndex = -1;
		return true;
	}
	int i = (int) (t * n / m_Duration);
	if (i >= n)
		return true;
	if (m_CurIndex != i)
		changed = true;
	m_CurIndex = i;
	if ((m_CurIndex >= 0) || (m_CurIndex < (int) m_Sources.GetSize()))
	{
		Morph* morph = FindMorph();
		VX_TRACE(Debug, ("MeshAnimator: %s t = %0.3f index = %d\n", GetName(), t, i));

		if (changed && (morph != NULL))
		{
			VertexArray* v1 = morph->GetSource(0);
			VertexArray* v2 = morph->GetSource(1);
			if (v1 && v2)
			{
				float w = t / m_Duration;

				morph->SetWeight(0, 1 - w);
				morph->MakeRelative(v1, m_ActiveLocs);
				m_ActiveLocs = m_Sources.GetAt(m_CurIndex);
				morph->SetWeight(1, w);
				morph->MakeRelative(v2, m_ActiveLocs);
				return true;
			}
		}
		m_ActiveLocs = m_Sources.GetAt(m_CurIndex);
		return Deformer::Eval(t);
	}
	return true;
}

/*
 * If a scene is loaded that contains a single vertex array,
 * use the name of the file to figure out which frame it
 * is in the sequence. If the base file name looks like
 *		xxx_nnn.vix
 * where <nnn> is a number, it is assumed to be the frame
 * number (starting at 0).
 */
bool MeshAnimator::OnEvent(Event* e)
{
	LoadSceneEvent*		le = (LoadSceneEvent*) e;
	SharedObj*		obj;
	const TCHAR*	name;
	const TCHAR*	num;
	int				n;

	if (e->Code != Event::LOAD_SCENE)
		return true;
	obj = (SharedObj*) le->Object;
	if (obj == NULL)
		return true;
	if (!obj->IsClass(VX_VtxArray))
		return true;
	name = obj->GetName();
	if (name == NULL)
		return true;
	num = STRCHR(name, TCHAR('_'));
	if (num == NULL)
		return true;
	n = ATOL(num);
	m_Sources.SetAt(n, (VertexArray*) obj);
	return true;
}

void MeshAnimator::SetTarget(SharedObj* obj)
{
	if (obj == NULL)
		DisconnectMeshes();
	else if (obj->IsClass(VX_Model))
		ConnectMeshes((Model*) obj);
	Deformer::SetTarget(obj);
}

void MeshAnimator::ConnectMeshes(Model* root)
{
	GroupIterNotSafe<Engine> iter(this, Group::DEPTH_FIRST);
	MeshAnimator* meshanim;

	while (meshanim = (MeshAnimator*) iter.Next())
		if (meshanim->IsClass(VX_MeshAnimator))
		{
			Core::String name(meshanim->GetName());
			Shape* tmp;
			Mesh* mesh;
			size_t	n;
			const TCHAR* lookfor;

			if (name.IsEmpty())
				continue;
			if (name.Right(9) == TEXT(".meshanim"))
			{
				n = name.GetLength();
				VX_ASSERT(n < INT_MAX);
				name = name.Left((int32) n - 9);
			}
			n = name.Find('.');
			if (n > 0)
				lookfor = (const TCHAR*) name + n;
			else
				lookfor = name;
			tmp = (Shape*) root->Find(lookfor, Group::FIND_END | Group::FIND_DESCEND);
			if ((tmp == NULL) || !tmp->IsClass(VX_Shape))
				continue;
			mesh = (Mesh*)  tmp->GetGeometry();
			if ((mesh == NULL) || !mesh->IsClass(VX_Mesh))
				continue;
			meshanim->Engine::SetTarget(mesh);
			VX_TRACE(Debug, ("MeshAnimator: %s -> shape %s\n", meshanim->GetName(), tmp->GetName()));
		}
}

void MeshAnimator::DisconnectMeshes()
{
	GroupIterNotSafe<Engine> iter(this, Group::DEPTH_FIRST);
	MeshAnimator* meshanim;

	while (meshanim = (MeshAnimator*) iter.Next())
		if (meshanim->IsClass(VX_MeshAnimator))
			meshanim->Engine::SetTarget(NULL);
}

}	// end Vixen