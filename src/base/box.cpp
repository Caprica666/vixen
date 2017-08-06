#include "vixen.h"

namespace Vixen
{
DebugOut& operator<<(DebugOut& s, const Box3& b)
{
	s << b.min.x << " " << b.min.y << " " << b.min.z << " ";
	s << b.max.x << " " << b.max.y << " " << b.max.z;
	return s;
}

/*!
 * @fn bool Box2::Contains(const Vec2& p) const
 * @param p point to check for containment
 *
 * A point is inside the box if it is smaller than or equal
 * to all of the box's dimensions.
 *
 * @return \b true if box contains the point, else \b false
 *
 * @see Box3::Around Box3::Contains
 */
bool Box2::Contains(const Vec2& p) const
{
	if ((p.x > max.x) || (p.x < min.x))
		return false;
	if ((p.y > max.y) || (p.y < min.y))
		return false;
	return true;
}



/*!
 * @fn void Box3::Normalize()
 * A normalized box has the \b min corner with the minimum X,Y,Z values
 * and the \b max corner with the maximum coordinate values.
 *
 * @see Box3::Set Box3::operator= Box3::Box3
 */
void Box3::Normalize()
{
	float tmp;

	if (min.x > max.x)
	   { tmp = min.x; min.x = max.x; max.x = tmp; }
	if (min.y > max.y)
	   { tmp = min.y; min.y = max.y; max.y = tmp; }
	if (min.z > max.z)
	   { tmp = min.z; min.z = max.z; max.z = tmp; }
}

/*!
 * @fn Box3& Box3::operator*=(const Matrix& matrix)
 *
 * The eight corners of the bounding volume are calculated
 * and transformed independently. A new axially aligned
 * bounding volume that encloses these points is stored in this box.
 *
 * @see Matrix Matrix::Transform
 */
Box3& Box3::operator*=(const Matrix& matrix)
{
	int i;
  	float	w = Width();
	float	h = Height();
    Vec3	pts[8];
/*
 * Get the box's 8 corners
 */
    pts[0] = min;
    pts[1].x = min.x + w;
    pts[1].y = min.y;
    pts[1].z = min.z;
    pts[2].x = pts[1].x;
    pts[2].y = min.y + h;
    pts[2].z = min.z;
    pts[3].x = min.x;
    pts[3].y = pts[2].y;
    pts[3].z = min.z;

    pts[4] = max;
    pts[5].x = max.x - w;
    pts[5].y = max.y;
    pts[5].z = max.z;
    pts[6].x = pts[5].x;
    pts[6].y = max.y - h;
    pts[6].z = max.z;
    pts[7].x = max.x;
    pts[7].y = pts[6].y;
    pts[7].z = max.z;
/*
 * Transform the 8 corners and compute a new box that encloses
 * the transformed points
 */
	for (i = 0; i < 8; i++)
		pts[i] *= matrix;
	min = pts[0];	/* initialize them to a valid values */
	max = pts[0];		
	Extend(pts + 1, 7);
	return *this;
}

Vec3 Box3::Center() const
{
	return Vec3((min.x + max.x) / 2,
				  (min.y + max.y) / 2,
				  (min.z + max.z) / 2);
}

bool Box3::operator==(const Box3& b2) const
	{ return ((min == b2.min) && (max == b2.max)); }

bool Box3::operator!=(const Box3& b2) const
	{ return ((min != b2.min) || (max != b2.max)); }

/*!
 * @fn Box3::Box3(const Vec3& p1, const Vec3& p2)
 * @param p1	first input point
 * @param p2	second input point
 *
 * Computes the smallest bounding box which encloses two input points.
 *
 * @see Box3::Around Box3::Extend
 */
Box3::Box3(const Vec3& p1, const Vec3& p2)
	{ Around(p1, p2); }

/*!
 * @fn Box3::Around(const Vec3& p1, const Vec3& p2)
 * @param p1	first input point
 * @param p2	second input point
 *
 * Make this the smallest bounding box which encloses the two points.
 *
 * @see Box3::Around Box3::Extend Box3::Contains
 */
void Box3::Around(const Vec3& p1, const Vec3& p2)
{
    min = p1;
    max = p1;
	Extend(p2);
}

// RAM_XX
#if 0	// Inlined in vxbox.inl
/*!
 * @fn Box3::Extend(const Vec3* points, int n)
 * @param points	array of one or more points
 * @param n			number of points in array (defaults to 1)
 *
 * Make this the smallest bounding box which encloses the two points.
 *
 * @see Box3::Around Box3::Contains Box3::Box3
 */
void Box3::Extend(const Vec3 *p, int n)
{
	if (IsNull())
	{
		min = *p;
		max = *p++;
		--n;
	}
		
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
#endif

/*!
 * @fn Box3::Extend(const Box3& src)
 * @param src	box to surround
 *
 * Computes the smallest bounding box which encloses the input box
 * and this box.
 *
 * @see Box3::Box3 Box3::Around
 */
void Box3::Extend(const Box3& src)
{
	if (src.IsNull())
		return;
	if (IsNull())
	{
		*this = src;
		return;
	}

    min.x = (min.x < src.min.x) ? min.x : src.min.x;
    min.y = (min.y < src.min.y) ? min.y : src.min.y;
    min.z = (min.z < src.min.z) ? min.z : src.min.z;
    max.x = (max.x > src.max.x) ? max.x : src.max.x;
    max.y = (max.y > src.max.y) ? max.y : src.max.y;
    max.z = (max.z > src.max.z) ? max.z : src.max.z;
}

/*!
 * @fn bool Box3::Contains(const Vec3& p) const
 * @param p		point to check for containment
 *
 * @return \b true if box contains the point, else \b false
 *
 * @see Box3::Around Box3::Hit
 */
bool Box3::Contains(const Vec3& p) const
{
	if ((p.x > max.x) || (p.x < min.x))
		return false;
	if ((p.y > max.y) || (p.y < min.y))
		return false;
	if ((p.z > max.z) || (p.z < min.z))
		return false;
	return true;
}

/*!
 * @fn float Box3::Hit(const Plane& plane)
 * @param plane	plane to test against
 *
 * Determines whether this box is inside the plane, outside the
 * plane or intersecting it. The two corners of the box along the diagonal
 * most closely aligned with the plane's normal are tested.
 *
 * @return signed number as follows
 * - 0 sphere intersects plane
 * - < 0 if sphere is on the side of the plane the normal points towards
 * - > 0 for the other side. Larger magnitudes indicate the sphere is further from the plane.
 *
 * @see Camera::IsVisible Sphere::Hit
 */
float Box3::Hit(const Plane& plane) const
{
	Vec3	n, p;
	int		cullmask = 0;
	float	dist;

	if (plane.x < 0)
		cullmask |= 1;
	if (plane.y < 0)
		cullmask |= 2;
	if (plane.z < 0)
		cullmask |= 4;
	switch (cullmask)		// select corners to test
	{
		case 0:				// +X +Y +Z
		p = this->max;
		n = this->min;
		break;				

		case 7:				// -X -Y -Z
		p = this->min;
		n = this->max;
		break;				

		case 1:				// -X +Y +Z
		p.Set(min.x, max.y, max.z);
		n.Set(max.x, min.y, min.z);
		break;

		case 6:				// +X -Y -Z
		p.Set(max.x, min.y, min.z);
		n.Set(min.x, max.y, max.z);
		break;

		case 2:				// +X -Y +Z
		p.Set(max.x, min.y, max.z);
		n.Set(min.x, max.y, min.z);
		break;

		case 5:				// -X +Y -Z
		p.Set(min.x, max.y, min.z);
		n.Set(max.x, min.y, max.z);
		break;

		case 3:				// -X -Y +Z
		p.Set(min.x, min.y, max.z);
		n.Set(max.x, max.y, min.z);
		break;

		case 4:				// +X +Y -Z
		p.Set(max.x, max.y, min.z);
		n.Set(min.x, min.y, max.z);
		break;
	}
	dist = plane.Distance(n);
	if (dist > 0)			// box completely outside plane
		return dist;
	dist = plane.Distance(p);
	if (dist <= 0)			// box completely inside plane
		return dist;
	return 0;				// box intersects plane
}

/*!
 * @fn bool Box3::Hit(const Ray &ray, float& distance, Vec3 *intersect) const
 * @param ray	ray to intersect, assumed to be in coordinate space of box
 * @param distance	where to store distance of closest intersection
 * @param intersect	where to store the intersection point.
 *
 * A ray has both a direction and a length. It will penetrate the
 * box if it pentrates one of the sides before it ends.
 *
 * @return \b true if ray hit the box, else \b false
 *
 *@see Sphere::Hit Model::Hit Box3::Contains Ray
 */
#define RIGHT	0
#define LEFT	1
#define MIDDLE	2

bool  Box3::Hit(const Ray &ray, float& distance, Vec3* intersect) const
{
	bool	inside = true;
	char	quadrant[3];
	int		i;
	int		whichPlane;
	Vec3	maxT;
	float	candidatePlane[3];
	Vec3	coord;

	/* Find candidate planes; this loop can be avoided if
   	rays cast all from the eye(assume perpsective view) */
	for (i = 0; i < 3; i++)
		if (ray.start[i] < min[i])
		{
			quadrant[i] = LEFT;
			candidatePlane[i] = min[i];
			inside = false;
		}
		else if (ray.start[i] > max[i])
		{
			quadrant[i] = RIGHT;
			candidatePlane[i] = max[i];
			inside = false;
		}
		else
			quadrant[i] = MIDDLE;

	/* Ray origin inside bounding box */
	if (inside)
	{
		coord = ray.start;
		distance = FLT_EPSILON;
		return true;
	}

	/* Calculate distances to candidate planes */
	for (i = 0; i < 3; i++)
		if (quadrant[i] != MIDDLE && ray.direction[i] != 0)
			maxT[i] = (candidatePlane[i] - ray.start[i]) / ray.direction[i];
		else
			maxT[i] = -1;

	/* Get largest of the maxT's for final choice of intersection */
	whichPlane = 0;
	for (i = 1; i < 3; i++)
		if (maxT[whichPlane] < maxT[i])
			whichPlane = i;

	/* Check final candidate actually inside box */
	if (maxT[whichPlane] < 0) return (false);
	for (i = 0; i < 3; i++)
		if (whichPlane != i)
		{
			coord[i] = ray.start[i] + maxT[whichPlane] * ray.direction[i];
			if (coord[i] < min[i] || coord[i] > max[i])
				return (false);
		}
		else 
			coord[i] = candidatePlane[i];
	
	if (intersect)
		*intersect = coord;
	distance = coord.Distance(ray.start);
	return (distance <= ray.length);
}

}	// end Vixen
