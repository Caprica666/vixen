/*!
 * @file vxbox.h
 * @brief 2D and 3D rectilinear bounding volumes.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxmath.h vxsphere.h
 */

#pragma once

namespace Vixen {

class Sphere;

/*!
 * struct Box2
 * @brief 2D rectangular bounding box represented in terms of
 * upper left and lower right corners in floating point coordinates.
 *
 * @ingroup vixen
 * @see Vec2 Box3
 */
struct Box2
{
	Vec2 min;		//!< minimum corner dimensions
	Vec2 max;		//!< maximum corner dimensions

//! Construct an empty box (both corners at origin)
	Box2() { }
//! Construct a box from minimum and maximum corner dimensions
	Box2(float xmin, float xmax, float ymin, float ymax)
		{ min.x = xmin; max.x = xmax; min.y = ymin; max.y = ymax; }
//! Determine whether the given point is inside this box.
 	bool Contains(const Vec2&) const;
//! Update a box from minimum and maximum corner dimensions
	void Set(float xmin, float xmax, float ymin, float ymax)
		{ min.x = xmin; max.x = xmax; min.y = ymin; max.y = ymax; }
//! Return the height of the box
	float Height() const
		{ return max.y - min.y; }
//! Return the width of the box
	float Width() const
		{ return max.x - min.x; }
//! Return the center of the box
	Vec2	Center() const
		{ return Vec2((min.x + max.x) / 2, (min.y + max.y) / 2); }
};


/*!
 * class Box3
 * @brief 3D rectilinear bounding box represented in terms of
 * upper left and lower right corners in floating point coordinates.
 *
 * @ingroup vixen
 * @see Vec3 Box2
 */
class Box3
{
public:
	Vec3	min;	//!< minimum corner dimensions
	Vec3	max;	//!< maximum corner dimensions

//! @name Construction
//!@{
//! Construct an uninitialized box
	Box3() { };
//! Construct this box from another
	Box3(const Box3&);
//! Construct a box from a 3D sphere
	Box3(const Sphere&);
//! Construct a box from two corners
	Box3(const Vec3&, const Vec3&);
//! Construct a box from minimum and maximum corner dimensions
	Box3(float xmin, float ymin, float zmin,
		float xmax, float ymax, float zmax);
//! Update a box from minimum and maximum corner dimensions
	void		Set(float xmin, float ymin, float zmin,
					float xmax, float ymax, float zmax);
//! Update a box from two corners
	void		Set(const Vec3&, const Vec3&);
//! Make box empty (both corners at 0)
	void		Empty();
//!@}

//! @name Operators
//!@{
//! Copy the source box into this box
	Box3&		operator=(const Box3&);
//! Make the box tightly enclose the input sphere
	Box3&		operator=(const Sphere&);
//! Multiply this box by a matrix
	Box3&		operator*=(const Matrix&);
//! Compare two boxes for equality
	bool		operator==(const Box3&) const;
//! Compare two boxes for inequality
	bool		operator!=(const Box3&) const;
//! Normalize box corners to be minimum, maximum
	void		Normalize();
//!@}

//! @name Accessors
//!@{
//! Return the width of the box
	float		Width() const;
//! Return the height of the box
	float		Height() const;
//! Return depth of box
	float		Depth() const;
//! Return the center of the box
	Vec3		Center() const;
//! Determine if box is empty (both corners are equal)
	bool		IsEmpty() const;
//! Determine if box is null (both corners are at the origin)
	bool		IsNull() const;
//!@}

//! @name Bounding
//!@{
//! Determine whether the given point is inside this box.
	bool		Contains(const Vec3&) const;

//! Make this box tighly enclose the input points.
	void		Around(const Vec3&, const Vec3&);

//! Make this box large enough to contain the input box.
	void		Extend(const Box3&);

//! Make this box large enough to contain another point.
	void		Extend(const Vec3&);

//! Make this box large enough to contain the input points.
	void		Extend(const Vec3*, int n = 1);

//! Determine if ray penetrates the box and where.
	bool		Hit(const Ray &l, float& closeDist, Vec3 *p) const;

//! Determine if box intersects plane
	float		Hit(const Plane& plane) const;
//!@}
};

DebugOut& operator<<(DebugOut& o, const Box3& b);

} // end Vixen