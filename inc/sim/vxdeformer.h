/*!
 * @file vxdeformer.h
 * @brief Base class for localized mesh deformation.
 *
 * Deforms selected vertices and normals in a mesh.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vvxskin.h vxmorph.h
 */
#pragma once

namespace Vixen {

class VertexCache;

/*!
 * @class Deformer
 * @brief Deforms a subset of the vertices and/or normals of a single mesh.
 *
 * The deformer is a common base class for mesh deformations like
 * skinning and morphing.
 *
 * @see Skin Morph
 * @ingroup vixen
 */
class Deformer : public Engine
{
public:
	VX_DECLARE_CLASS(Deformer);
	Deformer();

	void			SetRestLocs(VertexArray*);			//!< Set of locations corresponding to rest pose (undeformed).
	void			SetRestNormals(FloatArray*);		//!< Set normals corresponding to rest pose (undeformed).
	void			SetVertexMap(IntArray*);			//!< Set rest pose -> mesh mapping table.
	bool			MakeVertexMap(const VertexArray*);	//!< Compute a mapping between rest pose neutral locations and mesh vertices.
	IntArray*		GetVertexMap() const;				//!< Get rest pose -> mesh mapping table if one exists.
	VertexArray*	GetRestLocs() const;				//!< Get rest pose locations.
	FloatArray*		GetRestNormals() const;				//!< Get rest pose normals.
	FloatArray*		GetActiveLocs() const;				//!< Get unique active (deformed) locations.
	FloatArray*		GetActiveNormals() const;			//!< Get active (deformed) normals.
	VertexCache*	MakeVertexCache();
	bool			GetBound(Box3&) const;				//!< Return bounds of active locations

	virtual int		Save(Messenger&, int) const;
	virtual bool	Do(Messenger& s, int op);
	virtual bool	Copy(const SharedObj*);
	virtual bool	Init(SharedObj* target, VertexCache* vdict = NULL);
	virtual bool	Eval(float t);
	virtual void	SetTarget(SharedObj*);
	virtual	float	ComputeTime(float t);
	virtual	DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;
	static	IntArray*	ComputeVertexMap(const VertexArray* srcverts, const VertexArray* dstverts, VertexCache* vcache = NULL);
	static	bool		UpdateVertices(const FloatArray* srclocs, VertexArray* dstverts, const IntArray* vmap, const FloatArray* srcnmls);

	enum Opcode
	{
		DEFORM_SetSkeletonOLD = Engine::ENG_NextOp,
		DEFORM_SetRestLocs,
		DEFORM_SetMatricesOLD,
		DEFORM_SetVertexMap,
		DEFORM_SetRestNormals,
		DEFORM_Next = Engine::ENG_NextOp + 10
	};

protected:
	virtual VertexArray*	ValidateTarget(SharedObj* target);
	virtual	bool			Reset();
	virtual	int				InitRestNormals();

	Ref<IntArray>		m_VertexMap;
	Ref<VertexArray>	m_TargetVerts;
	Ref<Mesh>			m_TargetMesh;
	Ref<VertexArray>	m_RestLocs;
	Ref<FloatArray>		m_RestNormals;
	Ref<FloatArray>		m_ActiveLocs;
	Ref<FloatArray>		m_ActiveNormals;
	Ref<VertexCache>	m_VertexCache;
};

FORCE_INLINE IntArray* Deformer::GetVertexMap() const
	{ return m_VertexMap;	}

/*!
 * @fn VertexArray* Deformer::GetRestLocs() const
 * The rest locations define the neutral shape of the object with no deformations.
 * They also contain the bone indices and blend weights for each vertex.
 * Deformers compute their results relative to this neutral shape.
 * The entries in this array are unique and correspond to locations in
 * the target vertex array. These neutral locations are used as input by the
 * Deformer to update the vertex array each frame.
 *
 * @see Deformer::SetTarget Deformer::SetRestLocs Deformer::GetRestNormals Deformer::MakeVertexMap
 */
FORCE_INLINE VertexArray* Deformer::GetRestLocs() const
	{ return m_RestLocs;	}

/*!
 * @fn FloatArray* Deformer::GetRestNormals() const
 * The rest normals are the normals for the neutral shape of the object with no deformations.
 * Deformers compute their results relative to this neutral shape.
 * The entries in this array are not unique. They correspond directly to normals in
 * the target vertex array.
 *
 * @see Deformer::SetTarget Deformer::SetRestNormals Deformer::MakeVertexMap
 */
FORCE_INLINE FloatArray* Deformer::GetRestNormals() const
	{ return m_RestNormals; }

FORCE_INLINE FloatArray* Deformer::GetActiveLocs() const
	{ return m_ActiveLocs; }

FORCE_INLINE FloatArray* Deformer::GetActiveNormals() const
	{ return m_ActiveNormals; }

} // end Vixen


