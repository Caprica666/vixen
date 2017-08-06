/*!
 * @file vxsphere.h
 * @brief 3D bounding sphere.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxmath.h vxbox.h
 */
#pragma once

namespace Vixen {

/*!
 * @class Sphere
 * @brief 3D spherical bounding volume.
 *
 * It is used by the scene manager for keeping bounding spheres
 * to use for view volume culling.
 *
 * @ingroup vixen
 * @see Box3 Vec3 Model::GetBound Model::Hit
 */
class Sphere
{
public:
//! @name Construction
//!@{
//! Construct empty sphere (center at origin, radius 0)
    Sphere () {} ;
//! Construct smallest sphere which encloses the box.
    Sphere (const Box3&);
//! Construct one sphere from another.
    Sphere (const Sphere&);
//! Construct sphere from center and radius.
    Sphere (const Vec3& center, float radius);
//! Update center and radius of sphere.
    void	Set(const Vec3& center, float radius);
//! Make this an empty sphere  (center at origin, radius 0)
	void	Empty( );
//! Determine if this sphere is empty
	bool	IsEmpty() const;
//!@}

//! @name Bounding
//!@{
//! Extend this sphere to include the input sphere.
    void	Extend(const Sphere&);
//! Extend this sphere to include the input box.
    void	Extend(const Box3&);
//! Determine where sphere intersects plane.
	float	Hit(const Plane& plane) const;
//! Determine where input ray penetrates this sphere.
	bool	Hit(const Ray&, float& distance, Vec3* intersect) const;
//! Determine if input ray penetrates this sphere.
	bool	Hit(const Ray&) const;
//!@}

//! @name Operators
//!@{
//! Transform sphere by input matrix.
    Sphere&	operator*=(const Matrix&);
//! Copy input sphere into this sphere.
    Sphere&	operator=(const Sphere&);
//! Make this sphere tightly enclose the input box.
    Sphere&	operator=(const Box3&);
//! Test spheres for equality.
    bool		operator==(const Sphere&) const;
//! Test spheres for inequality.
    bool		operator!=(const Sphere&) const;
//!@}

    Vec3	Center;		//!< center of sphere
    float	Radius;		//!< radius of sphere
};

} // end Vixen