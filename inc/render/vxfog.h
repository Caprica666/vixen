#pragma once

namespace Vixen {

/*!
 * @class Fog
 * @brief Encapsulates the fogging characteristics of a scene.
 *
 * A fog object is attached to the Scene if fogging is desired.
 * Fog may be enabled and disabled at the shape level by setting
 *  APPEAR_Fogging in the appearance.
 *
 * Fog blends a fog color with each color after lighting and texturing
 * using a blending factor \b f. Factor \b f is computed in one of three ways,
 * depending on the kind of fog. Let  z be the distance in eye coordinates
 * from the camera to the pixel being fogged. Below ew show the
 * equations for each type of fog and how the fog blending factor is
 * calculated from them.
 *
 * - Linear			@image html fog_lin.jpg
 * - Exponential	@image html fog_exp.jpg
 * - Exponential2	@image html fog_exp2.jpg
 *
 * Regardless of the fog kind, \b f is clamped to the range [0,1]
 * after it is computed. Fog colors are either computed at each
 * pixel by the graphics display card or at the vertex level by the
 * geometry engine.
 * 
 * The fogged color  Cnew is computed from the fog color  Cfog and
 * the vertex or pixel color  C using the fog blending factor \b f:
 * @code
 *	Cnew = Cfog * f + (1 - f) * C
 * @endcode
 *
 * @ingroup vixen
 * @see Appearance::Set Scene::SetFog
 */
class Fog : public SharedObj
{
public:
	VX_DECLARE_CLASS(Fog);

//	Initializers
	Fog();								//!< Construct fog objec in default state.
	Fog(const Fog&);					//!< Construct one fog object from another.
	bool operator==(const Fog&);			//!< Return  true if fog objects are equal.
	bool operator!=(const Fog& src)		//!< Return  true if fog objects not equal.
		{ return !operator==(src); }

//	Material Attribute Accessors
	virtual void	SetStart(float);		//!< Set near distance for linear fog.
	float			GetStart() const;		//!< Get linear fog near distance.
	virtual void	SetEnd(float);			//!< Set far distance for linear fog.
	float			GetEnd() const;			//!< Get linear fog far distance.
	virtual void	SetDensity(float);		//!< Set fog density.
	float			GetDensity() const;		//!< Get fog density.
	virtual void	SetColor(const Col4&); //!< Set color of fog.
	const Col4&	GetColor() const;		//!< Get fog color.
	virtual void	SetKind(int);			//!< Set kind of fog model.
	int				GetKind() const;		//!< Get kind of fog model used.

//  Overrides
	virtual bool	Copy(const SharedObj* src);
	virtual bool	Do(Messenger&, int);
	virtual int		Save(Messenger&, int) const;
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*!
	 * @brief Fog kinds
	 * @see SetKind
	 */
	enum
	{
		LINEAR = 0,
		EXPONENTIAL,
		EXPONENTIAL2,
		VERTEX = 0,
		PIXEL = 4
	};

	/*
	 * Fog::Do opcodes (also for binary file format)
	 */
	enum Opcode
	{
		FOG_SetDensity = SharedObj::OBJ_NextOp,
		FOG_SetColor,
		FOG_SetStart,
		FOG_SetEnd,
		FOG_SetKind,
		FOG_NextOp = SharedObj::OBJ_NextOp + 20,
	};

protected:
	int32		m_Kind;				/* fog kind */
	Col4		m_Color;			/* fog color */
	float		m_Start;			/* start for linear fog */
	float		m_End;				/* end for linear fog */
	float		m_Density;			/* fog density */
};

inline float Fog::GetDensity() const
	{ return m_Density; }

inline const Col4& Fog::GetColor() const
	{ return m_Color; }

inline int Fog::GetKind() const
	{ return m_Kind; }

inline float Fog::GetStart() const
	{ return m_Start; }

inline float Fog::GetEnd() const
	{ return m_End; }

} // end Vixen