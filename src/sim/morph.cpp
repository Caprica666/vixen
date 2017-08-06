#include "vixen.h"

#ifndef VX_NOTHREAD
#include "computethread.h"
#endif

namespace Vixen {

VX_IMPLEMENT_CLASSID(Morph, Deformer, VX_Morph);

Morph::Morph() : Deformer()
{
#ifdef VX_USE_CILK
	m_Control |= Engine::TASK_PARALLEL | Engine::DATA_PARALLEL;
#else
	m_Control |= Engine::DATA_PARALLEL;
#endif
}

/*!
 * @fn void	Morph::SetWeight(int i, float weight)
 * @param i			0-based index of source blend shape
 * @param weight	blend weight between 0 and 1
 *
 * Establishes the blend weight to use for a given source.
 * To compute the target mesh vertices, the corresponding source
 * vertices are multiplied by their blend weight and summed.
 * The default blend weight if not specified is 0.
 *
 * @see Morph::SetSource
 */
void	Morph::SetWeight(int i, float w)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Morph, MORPH_SetWeight) << this << i << w;
	VX_STREAM_END(  )

	m_Weights.SetAt(i, w);
}


/*!
 * @fn void Morph::SetSource(int i, VertexArray* verts)
 * @param i		0-based index of source
 * @param verts array of vertices to use as input to deformation
 *
 * The input array references a set of vertices with the same ordering as
 * the target mesh. This source vertex array is multiplied by it's blend weight
 * to contribute to the computation of the target mesh vertices.
 * Setting a source to NULL effectively removes its contribution to the
 * target mesh. Setting the blend weight to 0 also does this.
 *
 * @see Morph::SetWeight
 */
void Morph::SetSource(int i, VertexArray* verts)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Morph, MORPH_SetSource) << this << i << verts;
	VX_STREAM_END(  )

	m_Sources.SetAt(i, verts);
}

bool Morph::MakeRelative(VertexArray* dest, const FloatArray* source)
{
	const VertexArray*	rest = GetRestLocs();
	intptr				nvtx;
	int					stride;
	float*				dstdata;
	const float*		restdata;
	const float*		srcdata;

	if ((dest == NULL) || (rest == NULL))
		return false;
	ObjectLock lock1(dest);
	ObjectLock lock2(rest);
	nvtx = dest->GetNumVtx();
	if (nvtx != rest->GetNumVtx())
		return false;
	stride = dest->GetVtxSize();
	if (source != NULL)
	{
		source->Lock(); 
		srcdata = source->GetData();
	}
	else srcdata = dest->GetData();
	restdata = rest->GetData();
	dstdata = dest->GetData();
	#pragma omp PARALLEL_FOR(nvtx)
	cilk_for (intptr j = 0; j < nvtx; ++j)
	{
		const Vec3*	srcvtx = (Vec3*) (srcdata + j * stride);
		const Vec3*	rstvtx = (Vec3*) (restdata + j * stride);
		Vec3*		dstvtx = (Vec3*) (dstdata + j * stride);

		*dstdata = *srcdata - *restdata;
	}
	if (source)
		source->Unlock();
	return true;
}

int Morph::Save(Messenger& s, int opts) const
{
	int32 h = Engine::Save(s, opts);
	int n;

	if (h < 0)
		return h;
	n = (int) m_Sources.GetSize();
	for (int i = 0; i < n; ++i)
	{
		const SharedObj* obj = m_Sources.GetAt(i);
		if (obj && (obj->Save(s, opts) >= 0) && h)
			s << OP(VX_Morph, MORPH_SetSource) << h << i << obj;
	}
	if (h == 0)
		return h;
	for (int i = 0; i < n; ++i)
		s << OP(VX_Morph, MORPH_SetWeight) << h << i << m_Weights.GetAt(i);
	return h;
}

bool Morph::Do(Messenger& s, int op)
{
	Matrix			mtx;
	float			v;
	int				i;
	SharedObj*		obj;

	switch (op)
	{
		case MORPH_SetSource:
		s >> i;
		s >> obj;
		SetSource(i, (VertexArray*) obj);
		break;

		case MORPH_SetWeight:
		s >> i >> v;
		SetWeight(i, v);
		break;


		default:
		return Deformer::Do(s, op);
	}

	return true;
}

DebugOut& Morph::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Deformer::Print(dbg, opts & ~PRINT_Trailer);
	if (opts & PRINT_Data)
	{
		endl(dbg << "<attr name='Weights'>");
		m_Weights.Print(dbg, opts & ~PRINT_Trailer);
		endl(dbg << "</attr>");
		endl(dbg << "<attr name='Sources'>");
		m_Sources.Print(dbg, opts & ~PRINT_Trailer);
		endl(dbg << "</attr>");
	}
	return Deformer::Print(dbg, opts & PRINT_Trailer);
}


bool Morph::Copy(const SharedObj* src_obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Deformer::Copy(src_obj))
		return false;
	const Morph* src = (const Morph*) src_obj;
	if (src->IsClass(VX_Deformer))
    {
		m_Weights.Copy(&(src->m_Weights));
		m_Sources.Copy(&(src->m_Sources));
   }
    return true;
}


void Morph::Compute(float t)
{
	if (!Init(m_Target))
	{
		Deformer::Compute(t);
		return;
	}
	VertexArray* dstlocs = m_TargetVerts;
	int			nshapes = (int) m_Sources.GetSize();
	ObjectLock	lock(dstlocs);
	for (int i = 0; i < nshapes; ++i)
	{
		VertexArray* verts = m_Sources[i];
		if (verts)
			verts->Lock();
	}
	Deformer::Compute(t);		// compute bone deformations
	for (int i = 0; i < nshapes; ++i)
	{
		VertexArray* verts = m_Sources[i];
		if (verts)
			verts->Unlock();
	}
}


bool Morph::Eval(float t)
{
	intptr	start = 0, n, total;
	int		debug = Deformer::Debug + Engine::Debug;

	if (m_TargetVerts.IsNull())
		return true;

	VertexArray*		dstlocs = m_TargetVerts;
	VertexArray::Iter	iter(dstlocs);
	intptr				nverts = dstlocs->GetNumVtx();
	int					nshapes = (int) m_Sources.GetSize();

	total = n = nverts;
	#pragma omp PARALLEL_FOR(n)
	cilk_for (intptr j = 0; j < n; ++j)
	{
		Vec3	vtx(0, 0, 0);
		intptr	vindex = j + start;
		Vec3*	dstvtx = (Vec3*) iter.GetLoc(vindex);

		for (int i = 0; i < nshapes; ++i)
		{
			const VertexArray*	srclocs = m_Sources.GetAt(i);
			float				f = m_Weights.GetAt(i);
			int					stride(srclocs->GetVtxSize());

			if ((f > 0.0f) && srclocs)
			{
				Vec3*	srcvtx = (Vec3*) (srclocs->GetData() + vindex * stride);
				if (f == 1.0f)
				{
					vtx.x = srcvtx->x;
					vtx.y = srcvtx->y;
					vtx.z = srcvtx->z;
				}
				else
				{
					vtx.x += f * srcvtx->x;
					vtx.y += f * srcvtx->y;
					vtx.z += f * srcvtx->z;
				}
			}
		}
		*dstvtx += vtx;
		++vindex;
	}
   return true;
}


}	// end Vixen