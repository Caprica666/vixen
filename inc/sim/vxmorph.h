/*!
 * @file vxmorph.h
 * @brief Base class for localized mesh deformation.
 *
 * Deforms selected vertices and normals in a mesh.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxbone.h vxskin.h vxmorph.h
 */
#pragma once

namespace Vixen {


/*!
 * @class Morph
 * @brief Deforms a subset of the vertices and normals of a single mesh based on a combination of source blend shapes.
 *
 * The morph references one or more vertex arrays with the same vertex ordering as the
 * target mesh. Each source arrays has an associated weight. The target mesh vertices are
 * computed by a weighted blend of the source vertex arrays.
 *
 * @image html skinmorph.gif
 *
 * @see Skin Transformer
 * @see @htmlonly <A><HREF="oview/animoview.htm">Vixen Animation Overview</A> @endhtmlonly
 * @ingroup vixen
 */
class Morph : public Deformer
{
public:
	VX_DECLARE_CLASS(Morph);
	Morph();

	//! Get the blend sources for the morph.
	RefArray<VertexArray>*	GetSources()		{ return &m_Sources; }

	//! Get the Ith glend source.
	VertexArray*	GetSource(int i) const		{ return m_Sources[i];	}

	// Set the Ith blend source.
	void			SetSource(int i, VertexArray* verts);

	//! Make vertex array relative to array of input locations
	bool			MakeRelative(VertexArray* dest, const FloatArray* source = NULL);

	//! Get blend weights array.
	FloatArray*		GetWeights()				{ return &m_Weights; }

	//! Get blend weight for the given source.
	float			GetWeight(int i) const		{ return m_Weights[i]; }

	//! Set blend weight for the given source.
	void			SetWeight(int i, float f);

	virtual int		Save(Messenger&, int) const;
	virtual bool	Do(Messenger& s, int op);
	virtual bool	Copy(const SharedObj*);
	virtual bool	Eval(float t);
	virtual	void	Compute(float t);
	virtual	DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	enum Opcode
	{
		MORPH_SetWeight = Deformer::ENG_NextOp,
		MORPH_SetSource,
		MORPH_Next = Deformer::DEFORM_Next + 5
	};

protected:
	RefArray<VertexArray>	m_Sources;
	FloatArray				m_Weights;
};


} // end Vixen


