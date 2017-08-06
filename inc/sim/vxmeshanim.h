/*!
 * @file vxmeshanim.h
 * @brief Base class for animating a sequence of meshes.
 *
 * Selects one of a series of meshes based on time.
 * Can be used to capture complex mesh simulations and play them back.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxbone.h vxskin.h vxmorph.h
 */
#pragma once

namespace Vixen {

class Skin;


/*!
 * @class MeshAnimator
 * @brief Animates a mesh using a sequence of vertex arrays over time.
 *
 * The mesh animator references one or more vertex arrays with the same vertex ordering as the
 * target mesh. They are assumed to be spaced equally in time. The mesh animator
 * selects one of the sources based on the current time and uses it to replace the
 * vertex array in the target mesh.
 *
 * @see Skin Deformer
 * @ingroup vixen
 */
class MeshAnimator : public Deformer
{
public:
	VX_DECLARE_CLASS(MeshAnimator);
	MeshAnimator();

	//! Get the Ith glend source.
	VertexArray*	GetSource(int i) const	{ return m_Sources[i];	}

	// Set the Ith blend source.
	void			SetSource(int i, VertexArray* verts);

	//! Get the time increment between sources.
	float			GetTimeInc() const		{ return m_TimeInc; }
	
	//! Get the number of sources.
	int				GetNumSources() const	{ return (int) m_Sources.GetSize(); }

	//! Set time increment between sources.
	void			SetTimeInc(float f);

	//! Clear all sources.
	void			Clear();
	
	virtual bool	Init(SharedObj* target, VertexCache* vdict = NULL);
	virtual int		Save(Messenger&, int) const;
	virtual bool	Do(Messenger& s, int op);
	virtual bool	Copy(const SharedObj*);
	virtual bool	Eval(float t);
	virtual bool	OnEvent(Event* e);
	virtual	bool	OnReset();
	virtual	void	SetTarget(SharedObj*);
	virtual	DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	enum Opcode
	{
		MESHANIM_SetSource = Engine::ENG_NextOp,
		MESHANIM_SetTimeInc,
		MESHANIM_Clear,
		MESHANIM_Next = Engine::ENG_NextOp + 10
	};

protected:
	virtual	bool	RecordTarget();			// record target mesh changes
	void			ConnectMeshes(Model*);	// connect hierarchy of MeshAnimators to similarly named Shapes
	void			DisconnectMeshes();		// disconnect hierarchy of MeshAnimators from target Shapes
	Morph*			FindMorph();			// find first child that is a Morph

	RefArray<VertexArray>	m_Sources;
	float					m_TimeInc;
	int						m_CurIndex;
};

} // end Vixen


