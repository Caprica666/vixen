/****
 *
 * Inlines for Box3 Class
 *
 ****/

namespace Vixen {

/*!
 * @fn Box3::Box3(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax)
 * @param xmin	minimum X value within box
 * @param ymin	minimum Y value within box
 * @param zmin	minimum Z value within box
 * @param xmax	maximum X value within box
 * @param ymax	maximum Y value within box
 * @param zmax	maximum Z value within box
 *
 * @see Box3::Set
 */
FORCE_INLINE Box3::Box3(float xmin, float ymin, float zmin,
				  float xmax, float ymax, float zmax)
	{ min.x = xmin; max.x = xmax; min.y = ymin; max.y = ymax;
	  min.z = zmin; max.z = zmax; }

/*!
 * @fn void Box3::Set(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax)
 * @param xmin	minimum X value within box
 * @param ymin	minimum Y value within box
 * @param zmin	minimum Z value within box
 * @param xmax	maximum X value within box
 * @param ymax	maximum Y value within box
 * @param zmax	maximum Z value within box
 *
 * Replaces the corners of this box with the input parameters
 * @see Box3::Box3
 */
FORCE_INLINE void Box3::Set(float xmin = 0.0f, float ymin = 0.0f,
					  float zmin = 0.0f, float xmax = 0.0f,
					  float ymax = 0.0f, float zmax = 0.0f)
	{ min.x = xmin; max.x = xmax; min.y = ymin; max.y = ymax;
	  min.z = zmin; max.z = zmax; }

/*!
 * @fn void Box3::Set(const Vec3& pmin, const Vec3& pmax)
 * @param pmin	minimum corner point
 * @param pmax	maximum corner point
 *
 * Replaces the corners of this box with the input parameters
 * @see Box3::Box3
 */
FORCE_INLINE void Box3::Set(const Vec3& pmin, const Vec3& pmax)
{
	min.x = pmin.x; max.x = pmax.x; min.y = pmin.y; max.y = pmax.y;
	min.z = pmin.z; max.z = pmax.z;
	Normalize();
}

/*!
 * @fn Box3::Box3(const Box3& box)
 * @param box	input box to copy
 *
 * The validity of the corner information in the
 * input box is not checked during construction.
 *
 * @see Box3::Set Box3::Normalize Box3::operator=
 */
FORCE_INLINE Box3::Box3(const Box3& box)
	{ min = box.min; max = box.max; }

/*!
 * @fn Box3& Box3::operator=(const Box3& box)
 * @param box	input box to copy
 *
 * The input box will be copied without checking the
 * validity of its corner information.
 *
 * @see Box3::Set Box3::Normalize Box3::Box3
 */
FORCE_INLINE Box3& Box3::operator=(const Box3& box)
	{ min = box.min; max = box.max; return *this; }

FORCE_INLINE float Box3::Width() const
	{ return max.x - min.x; }

FORCE_INLINE float Box3::Height() const
	{ return max.y - min.y; }

FORCE_INLINE float Box3::Depth() const
	{ return max.z - min.z; }

FORCE_INLINE bool Box3::IsEmpty() const
	{ return (min == max); }

FORCE_INLINE void Box3::Empty()
	{ min.Set(0,0,0); max.Set(0,0,0); }

FORCE_INLINE bool Box3::IsNull() const
{
	if (!IsEmpty())
		return false;
	if ((min.x != 0) || (min.y != 0) || (min.z != 0))
		return false;
	return true;
}

FORCE_INLINE void Box3::Extend(const Vec3& p)
	{ Extend(&p, 1); }

FORCE_INLINE Box3::Box3(const Sphere& sph)
{
	*this = sph;
}

/*!
 * @fn Box3::Extend(const Vec3* points, int n)
 * @param points	array of one or more points
 * @param n			number of points in array (defaults to 1)
 *
 * Make this the smallest bounding box which encloses the two points.
 *
 * @see Box3::Around Box3::Contains Box3::Box3
 */
FORCE_INLINE void Box3::Extend(const Vec3 *p, int n)
{
	while (--n >= 0)
	{
		if (p->x > max.x)		max.x = p->x;
		else if (p->x < min.x)	min.x = p->x;
		if (p->y > max.y)		max.y = p->y;
		else if (p->y < min.y)	min.y = p->y;
		if (p->z > max.z)		max.z = p->z;
		else if (p->z < min.z)	min.z = p->z;
		++p;
	}
}

} // end Vixen