/****
 *
 * Inlines for Model class
 *
 ****/

namespace Vixen {

/*
 * Internal model flags
 *
 * MOD_Invisible	TRUE if the model should not be displayed.
 * MOD_BVinvalid	TRUE if the model has invalid bounding volume
 *					(bounding sphere needs to be recomputed)
 * MOD_STinvalid	TRUE if the face and vertex counts are invalid
 */
#define	MOD_STinvalid	(OBJ_LASTUSED<<2)	// vertex, face count statistics not valid
#define	MOD_BVinvalid	(OBJ_LASTUSED<<1)	// bounding volume not valid
#define	MOD_Invisible	INACTIVE		// model should not be displayed

inline bool Model::WasRendered() const
	{ return m_Rendered; }

inline void Model::SetRendered(bool flag) const
	{ m_Rendered = flag; }

inline bool Model::IsCulling() const
	{ return !m_NoCull; }

inline int Model::GetHints() const
	{ return m_Hints; }

inline const Matrix* Model::GetTransform() const
{
	if (m_Transform.IsNull())
		return Matrix::GetIdentity();
	else return m_Transform;
}

inline intptr Model::GetNumVtx() const
{
	if (m_Verts > 0)
		return m_Verts;
	CalcStats();
	return m_Verts;
}

} // end Vixen