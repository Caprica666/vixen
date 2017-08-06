#pragma once

namespace Vixen {

/*!
 * @class BillBoard
 * @brief A BillBoard is a Shape that always faces the viewer.
 *
 * You can supply your own geometry and appearances or you can
 * use the default ones. The default Billboard geometry is a
 * single triangle strip representing a rectangular area with
 * coordinates between -0.5 and 0.5. A default appearance that is
 * uninteresting is supplied but you will probably want to
 * map a texture onto the billboard.
 *
 * There are two types of billboard rotation -  axial and  radial.
 * An  axial billboard has an axis of symmetry (a coordinate axis)
 * about which it rotates to face the viewer. A  radial billboard
 * does not rotate around an axis to face the viewer, it keeps
 * its normal vector always towards the viewer.
 *
 * To scale or move the billboard within your scene, use its local matrix.
 * Billboards will move and change size but they will not rotate.
 *
 * The texture associated with a BillBoard is kept in the
 * appearance array of the Shape.
 * The image associated with the BillBoard appearance can be
 * a single texture, an animated texture or a text image.
 *
 *  Multi-textured billboards select among a set of textures
 * that represent different views of the same object using the
 * angle between the object and the viewer. The texture set is
 * represented as an ImageSwitch object, which lets you select
 * one from among a list of images to use as a texture. If
 * the appearance used by the billboard's geometry has an
 * ImageSwitch as its image, this billboard can become multi-textured.
 * The order of the images in a multi-textured billboard is clockwise
 * around the object starting at the front.
 *	
 * @see ImageSwitch SimpleShape Sprite
 */
class BillBoard : public SimpleShape
{
public:
	VX_DECLARE_CLASS(BillBoard);
	BillBoard();

	void			SetKind(int);			//!< Set the kind of billboarding method to use. 
	int				GetKind() const;		//!< Get billboard kind.
	const Vec3&	GetAxis() const;		//!< Get billboard local rotation axis.
	void			SetAxis(const Vec3&);	//!< Set the axis of rotation for an axial billboard.
	float			GetAngle() const;		//!< Get angle between billboard and viewer.
	const Vec3&	GetNormal() const;		//!< Get billboard face normal.
	void			SetNormal(const Vec3&); //!< Set the face normal.
	static Vec3	ComputeNormal(const Box3&); //!< Compute face normal from bounding box.

	virtual bool	Copy(const SharedObj*);
	virtual bool	Do(Messenger& s, int op);
	virtual int		Save(Messenger&, int) const;
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*!
	 * @brief billboard rotation type
	 * @see BillBoard::SetKind
	 */
	enum
	{
		AXIAL = 0,		//!< rotates about single axis
		RADIAL = 1,		//!< face normal points at viewer
	};

	/*
	 * BillBoard::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		BB_SetKind = SHAPE_NextOp,
		BB_SetAxis,
		BB_SetNormal,
		BB_NextOp = SHAPE_NextOp + 20,
	};

protected:
	Geometry*		MakeImageGeometry();
	TextGeometry* MakeTextGeometry();

	bool			CalcMatrix(Matrix*, Scene* scene) const;
	void			AxialMatrix(Matrix*, Scene* scene) const;
	void			RadialMatrix(Matrix*, Scene* scene) const;
	void			CalcImageIndex() const;
	
	int		m_Kind;
	Vec3	m_Axis;
	Vec3	m_Normal;
	float	m_RotateAngle;
};


/*!
 * @fn float BillBoard::GetAngle() const
 *
 * Returns the rotation angle of the billboard with respect
 * to the line of sight. This might be useful for subclasses
 * which want to do some angle-dependent processing.
 * The angle is recomputed by BillBoard::CalcMatrix each frame
 */
inline float BillBoard::GetAngle() const
{
	return m_RotateAngle;
}

inline int BillBoard::GetKind() const
{	return m_Kind; }

inline const Vec3& BillBoard::GetAxis() const
{	return m_Axis; }

inline const Vec3& BillBoard::GetNormal() const
{	return m_Normal; }

inline BillBoard::BillBoard() : SimpleShape()
{
	m_Kind = RADIAL;
	m_Axis.Set(0.0f, 1.0f, 0.0f);
	m_Normal.Set(0.0f, 0.0f, 1.0f);
	SetHints(DYNAMIC);
}

} // end Vixen