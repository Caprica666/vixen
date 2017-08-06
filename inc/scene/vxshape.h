#pragma once
/*!
 * @file vxshape.h
 * @brief 3D object that binds geometry and appearance.
 *
 * A Shape is a node in the scene graph that may be rendered.
 * It combines material and lighting properites with geometric form.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxmodel.h vxappearance.h vxmesh.h vxscene.h
 */

namespace Vixen {

/*!
 * @class Shape
 *
 * @brief Model described in terms of geometric surfaces
 * that can have texture and material properties.
 *
 * Usually, shapes are used to represent objects whose basic form does not change,
 * like buildings in a city, the walls of a maze, or cars. You can
 * still move parts of a geometric object relative to one another,
 * like turning the wheels of a car, if you break them up into
 * multiple shapes and put them in a hierarchy.
 *
 * The geometric data for shapes can be derived from modeling programs,
 * specialized tools or tun-time construction. Their output is
 * accumulated into a  surface as a set of triangle meshes that
 * reference outside appearance properties (like textures and/or materials).
 * The same geometry can be used with different sets of appearance
 * properties. For example, the same set of shapes to define a human
 * baseball player can be displayed with different textures and materials
 * to represent different teams.
 *
 * @image html shapes.png
 *
 * @ingroup vixen
 * @see Appearance TriMesh
 */
class Shape : public Model
{
public:
	VX_DECLARE_CLASS(Shape);

	//! Construct empty shape with no geometry or appearances
	Shape() : Model() { };

	//! Construct shape that is a copy of another shape
	Shape(const Shape& s): Model() { Copy(&s); }

	//! Get the geometry to render
	Geometry*			GetGeometry();
	const Geometry*		GetGeometry() const;

	//! Set the geometry to render
	void				SetGeometry(const Geometry*);

	//! Get the appearance to use for shading
	Appearance*			GetAppearance();
	const Appearance*	GetAppearance() const;

	//! Set the appearance to use for shading
	void				SetAppearance(const Appearance*);

	//! Calculate rendering statistics
	virtual	void		CalcStats() const;
	virtual	void		CalcStats(intptr& numverts, intptr& numfaces) const;

//	Overrides
	virtual void		SetHints(int hints, bool descend = false);
	virtual bool		Do(Messenger& s, int opcode);
	virtual int			Save(Messenger&, int) const;
	virtual bool		Copy(const SharedObj*);
	virtual void		Render(Scene*);
	virtual bool		CalcBound(Box3*) const;
	virtual bool		CalcSphere(Sphere*) const;
	virtual intptr		Cull(const Matrix*, Scene*);
	virtual	DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*
	 * Shape::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		SHAPE_SetAppearance = Model::MOD_NextOp,
		SHAPE_SetGeometry,

	// SimpleShape opcodes
		SHAPE_SetImage,
		SHAPE_SetText,
		SHAPE_SetForeColor,
		SHAPE_SetTextFont,
		SHAPE_NextOp = Model::MOD_NextOp + 10
	};
protected:
//	Data members
	Ref<Geometry>		m_Geometry;
	Ref<Appearance>	m_Appearance;
   };

inline const Appearance* Shape::GetAppearance() const
	{ return m_Appearance; }

inline const Geometry* Shape::GetGeometry() const
	{ return m_Geometry; }

inline Appearance* Shape::GetAppearance()
	{ return m_Appearance; }

inline Geometry* Shape::GetGeometry()
	{ return m_Geometry; }

} // end Vixen