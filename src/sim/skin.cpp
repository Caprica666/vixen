#include "vixen.h"

#ifndef VX_NOTHREAD
#include "computethread.h"
#endif

namespace Vixen {

VX_IMPLEMENT_CLASSID(Skin, Deformer, VX_Skin);

/*!
 * @fn Skin::Skin()
 * @param numbones	 number of bones used for skinning.
 */
Skin::Skin() : Deformer()
{
	m_BlendWeightOfs = m_BlendIndexOfs = m_NormalOfs = -1;
	m_BonesPerVtx = 0;
#ifdef VX_USE_CILK
	m_Control |= Engine::TASK_PARALLEL | Engine::DATA_PARALLEL;
#else
	m_Control |= Engine::DATA_PARALLEL;
#endif
	m_Initialized = false;
}

Skin::~Skin()
{
	m_Skeleton = NULL;
}


/*!
 * @fn void Skin::SetSkeleton(Skeleton* skel)
 * @param skel	Skeleton to control this skin
 *
 * The skin's pose is the set of transformations which bring the bones from the bind pose of the skin
 * mesh to the desired position and orientation. Each vertex in the skin can be influenced by one or more bones.
 * The matrix associated with the bone is used to transform each neutral vertex that the bone influences.
 * The source vertices of the skin should contain bone indices compatible with the input pose for
 * each of the bone weights.
 *
 * Typically, the pose for skinning comes from a Skeleton updates as the skeleton changes.
 *
 * @see Deformer::SetRestPose Skeleton::GetSkinPose Skeleton
 */
void Skin::SetSkeleton(Skeleton* skel)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Skin, SKIN_SetSkeleton) << this << skel;
	VX_STREAM_END(  )

	m_Skeleton = skel;
	if (skel)
		m_Matrices.SetSize(skel->GetNumBones());
	else
		m_Matrices.SetSize(0);
}


/*!
 * @fn bool Skin::Init()
 * Initialize for skinning: find bone weights and matrix indices, create active verts array.
 */
bool Skin::Init(SharedObj* target, VertexCache* vdict)
{
	Shape*	shape = (Shape*) target;
	intptr	nvtx = 0;
	const DataLayout* layout;

	if (m_Initialized)
		return Reset();
	if (m_TargetVerts.IsNull())
		ValidateTarget(target);	
	if (m_RestLocs.IsNull())
		return false;
	nvtx = m_RestLocs->GetNumVtx();
	if (nvtx == 0)
		return false;
	/*
	 * Use the layout of the rest pose to determine the offset
	 * with the vertex of the blend weights and matrix ids.
	 */
	if (m_BonesPerVtx <= 0)
	{
		layout = m_RestLocs->GetLayout();
		VX_ASSERT(layout);
		for (intptr i = 0; i < (intptr) layout->NumSlots; ++i)
		{
			const LayoutSlot& slot = layout->Slot[i];

			if (slot.Name.Find(TEXT("weight")) >= 0)
			{
				m_BlendWeightOfs = slot.Offset;
				VX_ASSERT((m_BonesPerVtx == 0) || (m_BonesPerVtx == slot.Size));
				m_BonesPerVtx = slot.Size;
			}
			else if (slot.Name.Find(TEXT("blendindex")) >= 0)
			{
				m_BlendIndexOfs = slot.Offset;
				m_BonesPerVtx = slot.Size;
				VX_ASSERT(slot.Style & DataLayout::INTEGER);
			}
			else if (slot.Style & VertexPool::NORMALS)
				m_NormalOfs = slot.Offset;
		}
		Validate(0, nvtx);
	}
	/*
	 * Construct the vertex array with positions (and maybe normals) that are deformed by this skin.
	 * These vertices change every frame.
	 */
	nvtx *= 3;
	if (m_ActiveLocs.IsNull() || (m_ActiveLocs->GetSize() != nvtx))
	{
		m_ActiveLocs = new FloatArray(nvtx);
		m_ActiveLocs->SetSize(nvtx);
	}
	if (!Deformer::Init(NULL, vdict))
		return false;
	if (!m_RestNormals.IsNull() &&
		(m_ActiveNormals.IsNull() || (m_ActiveNormals->GetSize() != m_RestNormals->GetSize())))
		m_ActiveNormals = (FloatArray*) m_RestNormals->Clone();
	m_Initialized = true;
	return true;
}


/*!
 * @fn void Skin::DeformLocs(intptr start, intptr nvtx)
 * @param start	0-based index of starting vertex.
 * @param nvtx	number of vertices to deform.
 *
 * Deform locations based on the current matrix palette.
 *
 * @see Skin::SetMatrices Skin::Eval Skin::DeformLocsAndNormals
 */
void Skin::DeformLocs(intptr start, intptr nvtx)
{
	VertexArray::ConstIter	srciter(m_RestLocs);
	ObjectLock				lock((SharedObj*) m_RestLocs);
	int						max_mtx = (int) m_Matrices.GetSize();
	int						numbones = m_BonesPerVtx;
	const Array<Matrix>&	matrices = m_Matrices;
	float*					activelocs = m_ActiveLocs->GetData();

	VX_ASSERT(max_mtx > 0);
	nvtx += start;
	#pragma omp PARALLEL_FOR(nvtx)
	cilk_for (intptr i = start; i < nvtx; ++i)
	{
		const Vec3*		srcptr = srciter.GetLoc(i);
		const float*	wptr = ((const float*) srcptr) + m_BlendWeightOfs;
		const int32*	iptr = ((const int32*) srcptr) + m_BlendIndexOfs;
		Vec3*			dstptr = (Vec3*) (activelocs + i * 3);

		for (int b = 0; b < numbones; ++b)			// for each bone
		{
			int		mtx_index = iptr[b];			// get matrix index for this bone
			float	w = wptr[b];

			if (mtx_index < 0)
				break;
			if (w > 0.0f)							// negative index means end
			{
				const Matrix& mtx = matrices.GetAt(mtx_index);
				VX_ASSERT(mtx_index < max_mtx);
				if (!mtx.IsIdentity())				// identity matrix does nothing
				{
					Vec3	tmp;

					mtx.Transform(*srcptr, tmp);	// apply transform to neutral location
					tmp -= *srcptr;					// vertex deformation relative to neutral location
					tmp *= wptr[b];					// apply weight
					*dstptr += tmp;					// add to active locations
				}
			}
		}
	}
}

/*!
 * @fn void Skin::DeformLocsAndNormals(intptr start, intptr nvtx)
 * @param start	0-based index of starting vertex.
 * @param nvtx	number of vertices to deform.
 *
 * Deform locations and normals based on the current matrix palette.
 *
 * @see Skin::SetMatrices Skin::Eval Skin::DeformLocs
 */
void Skin::DeformLocsAndNormals(intptr start, intptr nvtx)
{
	VertexArray::ConstIter	srciter(m_RestLocs);
	ObjectLock				lock((SharedObj*) m_RestLocs);
	int						max_mtx = (int) m_Matrices.GetSize();
	int						numbones = m_BonesPerVtx;
	const Array<Matrix>&	matrices = m_Matrices;
	float*					activelocs = m_ActiveLocs->GetData();
	float*					activenmls = m_ActiveNormals->GetData();

	VX_ASSERT(max_mtx > 0);
	nvtx += start;
	#pragma omp PARALLEL_FOR(nvtx)
	cilk_for (intptr i = start; i < nvtx; ++i)
	{
		intptr			t = i * 3;
		const Vec3*		srcptr = srciter.GetLoc(i);
		const Vec3*		srcnml = (const Vec3*) srciter.GetNormal(i);
		Vec3*			dstptr = (Vec3*) (activelocs + t);
		Vec3*			dstnml = (Vec3*) (activenmls + t);
		const float*	wptr = ((const float*) srcptr) + m_BlendWeightOfs;
		const int32*	iptr = ((const int32*) srcptr) + m_BlendIndexOfs;
		float			wtot = 0.0f;
		Vec3			tmp;

		for (int b = 0; b < numbones; ++b)			// for each bone
		{
			int		mtx_index = iptr[b];			// get matrix index for this bone
			float	w = wptr[b];

			if (mtx_index < 0) 						// negative index signals end
				break;
			if (w > 0.0f)
			{
				VX_ASSERT(mtx_index < max_mtx);
				VX_ASSERT(fabs(w) < (1.0f + VX_EPSILON));

				const Matrix& mtx = matrices.GetAt(mtx_index);
				if (!mtx.IsIdentity())					// identity matrix does nothing
				{
					mtx.Transform(*srcptr, tmp);		// apply transform to neutral location
					tmp -= *srcptr;						// make relative to neutral location
					tmp *= w;							// apply weight
					*dstptr += tmp;						// add to active locations
					mtx.TransformVector(*srcnml, tmp);	// apply transform to neutral normal
					tmp -= *srcnml;						// make relative to neutral normal
					tmp *= w;							// apply weight
					*dstnml += tmp;						// add to active normals
				}
				wtot += w;								// total weight
			}
		}
	}
}

/*!
 * @fn void Skin::DeformNormals(intptr start, intptr nvtx)
 * @param start	0-based index of starting normal.
 * @param nvtx	number of normals to deform.
 *
 * Deform normals based on the current matrix palette.
 *
 * @see Skin::SetMatrices Skin::Eval Skin::DeformLocs Skin::DeformLocsAndNormals
 */
void Skin::DeformNormals(intptr start, intptr nvtx)
{
	ObjectLock				lock((SharedObj*) m_RestNormals);
	int						max_mtx = (int) m_Matrices.GetSize();
	int						numbones = m_BonesPerVtx;
	const Array<Matrix>&	matrices = m_Matrices;
	float*					activenmls = m_ActiveNormals->GetData();
	float*					srcnmls = m_RestNormals->GetData();

	VX_ASSERT(max_mtx > 0);
	nvtx += start;
	#pragma omp PARALLEL_FOR(nvtx)
	cilk_for (intptr i = start; i < nvtx; ++i)
	{
		intptr			t = i * 3;
		const Vec3*		srcnml = (const Vec3*) (srcnmls + t);
		Vec3*			dstnml = (Vec3*) (activenmls + t);
		Vec3			tmp;
		int				vindex = m_VertexMap->GetAt(i);
		const float*	wptr = GetBoneWeights(vindex);;
		const int32*	iptr = GetBoneIndices(vindex);

		for (int b = 0; b < numbones; ++b)			// for each bone
		{
			int		mtx_index = iptr[b];			// get matrix index for this bone
			float	w = wptr[b];

			if (mtx_index < 0)						// negative index signals end
				break;
			if (w > 0.0f)							// negative index signals end
			{
				VX_ASSERT(mtx_index < max_mtx);
				VX_ASSERT(fabs(w) < (1.0f + VX_EPSILON));

				const Matrix& mtx = matrices.GetAt(mtx_index);
				if (!mtx.IsIdentity())					// identity matrix does nothing
				{
					mtx.TransformVector(*srcnml, tmp);	// apply transform to neutral normal
					tmp -= *srcnml;						// make relative to neutral normal
					tmp *= w;							// apply weight
					*dstnml += tmp;						// add to active normals
				}
			}
		}
	}
}

/*
 * Validates the blend indices and weights.
 * If the weights for a vertex do not add up to 1.0, they are
 * normalized to make this the case.
 */
void Skin::Validate(intptr start, intptr nvtx) const
{
	VertexArray::ConstIter	srciter(m_RestLocs);
	int						numbones = m_BonesPerVtx;
	int						maxbones = GetNumBones();

	VX_TRACE(Debug, ("Skin::Validate %s\n", GetName()));
	if (nvtx == 0)
		nvtx = m_RestLocs->GetNumVtx();
	else
		nvtx += start;
	for (intptr i = start; i < nvtx; ++i)
	{
		const Vec3*		srcptr = srciter.GetLoc(i);
		float*			wptr = ((float*) srcptr) + m_BlendWeightOfs;
		const int32*	iptr = ((const int32*) srcptr) + m_BlendIndexOfs;
		float			total = 0.0f;
		int				b;

		for (b = 0; b < numbones; ++b)			// for each bone
		{
			int		mtx_index = iptr[b];		// get matrix index for this bone
			float	w = wptr[b];

			if (mtx_index < 0)					// negative index signals end
				break;
			total += w;
			if ((maxbones > 0) && (mtx_index >= maxbones))
			{
				VX_TRACE(Debug, ("\t vertex %d has bone index %d > max bones %d\n", i, mtx_index, maxbones));
				continue;
			}
		}
		if (fabs(total - 1.0f) < VX_EPSILON)
			continue;
		if (total <= VX_EPSILON)				// empty weights?
			continue;
		VX_TRACE(Debug, ("\tvertex %d has weight %f != 1\n", i, total));
		for (b = 0; b < numbones; ++b)			// for each bone
			if (iptr[b] >= 0)					// negative index signals end
				wptr[b] /= total;				// redistribute weights
	}
}

/*!
 * @fn bool Skin::Eval(float t)
 * Compute the new locations and normals from the bone transformations
 * for a subset of the vertices in the skin.
 *
 * @see Skin::Eval Skin::DeformLocs Skin::DeformLocsAndNormals
 */
bool Skin::Eval(float t)
{
	intptr	total = m_RestLocs->GetNumVtx();
	intptr	start = 0, nlocs = total;
	int	debug = Deformer::Debug + Engine::Debug + Skin::Debug;

	if (m_ActiveNormals.IsNull())
		DeformLocs(start, nlocs);
	else if (m_VertexMap.IsNull() && (m_RestLocs->GetStyle() & VertexPool::NORMALS))
		DeformLocsAndNormals(start, nlocs);
	else
	{
		DeformLocs(start, nlocs);
		if (!m_RestNormals.IsNull())
			DeformNormals(start, m_RestNormals->GetSize() / 3);
	}
	return true;
}


/*!
 * @fn bool Skin::Reset()
 * Update the local matrix stack and copy the source vertices (neutral positions & normals) into the active verts.
 */
bool Skin::Reset()
{
	Skeleton* skel = GetSkeleton();

	if (m_RestNormals.IsNull() && !m_VertexMap.IsNull() && (m_TargetVerts->GetStyle() & VertexPool::NORMALS))
		InitRestNormals();		// only needed if normals requested
	Deformer::Reset();
	if (skel == NULL)
		return false;
	/*
	 * Update the internal matrix stack from the current pose.
	 */
	skel->Lock();
	const Pose*	pose = skel->GetSkinPose();
	int			numbones = skel->GetNumBones();
	int			nchanged = 0;

	m_Matrices.SetSize(numbones);
	for (int i = 0; i < numbones; ++i)
	{
		Matrix& mtx(m_Matrices[i]);

		pose->GetWorldMatrix(i, mtx);
		if (!mtx.IsIdentity())
			nchanged++;
	}
	skel->Unlock();
	if (nchanged == 0)
		return false;
	SetChanged(true);
 	return true;
}


/*
 * Override Compute() so we can update target after children deform the verts
 */
void Skin::Compute(float t)
{
	Deformer::Compute(t);
	if (HasChanged() && !m_TargetVerts.IsNull())
	{
		Deformer::Eval(t);
		SetChanged(false);
	}
}


bool Skin::Do(Messenger& s, int op)
{
	SharedObj*	obj;

	switch (op)
	{
		case SKIN_SetSourceVertsOLD:
        s >> obj;
		VX_ASSERT(obj->IsClass(VX_VtxArray));
		SetRestLocs((VertexArray*) obj);
        break;

		case SKIN_SetVertexMapOLD:
        s >> obj;
		VX_ASSERT(obj->IsClass(VX_Array));
		SetVertexMap((IntArray*) obj);
        break;

		case SKIN_SetSkeletonOLD:
		case SKIN_SetSkeleton:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Skeleton));
		SetSkeleton((Skeleton*) obj);
		break;

		default:
		return Deformer::Do(s, op);
    }

    return true;
}

int Skin::Save(Messenger &s, int opts) const
{
	int32 h = Deformer::Save(s, opts);

	if (h < 0)
		return h;
	if (!m_Skeleton.IsNull() && h && ((const Engine*) m_Skeleton != Parent()))
	{
		if (m_Skeleton->Save(s, opts) >= 0)
			s << OP(VX_Skin, SKIN_SetSkeleton) << h << (const Skeleton*) m_Skeleton;
	}
	return h;
}

DebugOut& Skin::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return Deformer::Print(dbg, opts);
	Deformer::Print(dbg, opts & ~PRINT_Trailer);
	if (!m_Skeleton.IsNull())
	{
		endl(dbg << "<attr name='Skeleton'>");
		m_Skeleton->Print(dbg, PRINT_Summary);
		endl(dbg << "</attr>");
	}
	return Deformer::Print(dbg, opts & PRINT_Trailer);
}

bool Skin::Copy(const SharedObj* src_obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Deformer::Copy(src_obj))
		return false;
	const Skin* src = (const Skin*) src_obj;
	if (src->IsClass(VX_Skin))
		m_Skeleton = src->m_Skeleton;
    return true;
}


}	// end Vixen