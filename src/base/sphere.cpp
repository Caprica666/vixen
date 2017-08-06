#include "vixen.h"

namespace Vixen {

Box3& Box3::operator=(const Sphere& sph)
{
	Vec3	v(sph.Radius, sph.Radius, sph.Radius);
	min = sph.Center - v;
	max = sph.Center + v;
	return *this;
}

Sphere& Sphere::operator=(const Sphere& src)
{ 
    Center = src.Center;
    Radius = src.Radius;
    return *this; 
}

Sphere& Sphere::operator=(const Box3& src)
{ 
    Center = src.Center();
    Radius = Center.Distance(src.min);
	return *this;
}

void Sphere::Empty()
{
	Center.Set(0.0f, 0.0f, 0.0f);
	Radius = 0.0f;
}

bool Sphere::IsEmpty() const
{
	if (Radius > 0.0f || Center.x != 0.0f ||
		Center.y != 0.0f || Center.z != 0.0f)
		return false;
	return true;
}

Sphere::Sphere (const Sphere& src)
{   
    Center = src.Center ;
    Radius = src.Radius ;
}

Sphere::Sphere (const Vec3& cntr, float rds)
{ 
    Center = cntr ;
    Radius = rds ; 
}

Sphere::Sphere (const Box3& src)
	{ *this = src; }

void Sphere::Set (const Vec3& cntr, float rds)
{ 
    Center = cntr;
    Radius = rds; 
}

bool Sphere::operator == (const Sphere& src) const
{ 
    return ((Center == src.Center) && (Radius == src.Radius)); 
}

bool Sphere::operator != (const Sphere& src) const
{ 
    return ((Center != src.Center) || (Radius != src.Radius)); 
}

Sphere& Sphere::operator *= (const Matrix& trans)
{
	Sphere tmp(*this);
	trans.Transform(tmp, *this);
    return *this;
}

void Sphere::Extend(const Sphere& src)
{
    Vec3 Vec;

	if (src.IsEmpty())
		return;
	if (IsEmpty())
	{
		Set(src.Center, src.Radius);
		return;
	}
    Vec.x = src.Center.x - Center.x;
    Vec.y = src.Center.y - Center.y;
    Vec.z = src.Center.z - Center.z;
    float fDistance = Vec.Length();
    
   // BoundGeom enclose src
    if (Radius >= fDistance + src.Radius)
        return;

   // src enclose BoundGeom
    if (src.Radius >= fDistance + Radius) {
        Set(src.Center, src.Radius);
        return;
    }

   // BoundGeom not enclose src & src not enclose BoundGeom
    float fAlpha = (src.Radius - Radius + fDistance) / (2 * fDistance);
    Vec3 ptNewCenter;
    ptNewCenter.x = Center.x + fAlpha * Vec.x;
    ptNewCenter.y = Center.y + fAlpha * Vec.y;
    ptNewCenter.z = Center.z + fAlpha * Vec.z;
    float fNewRadius = 0.5f * (Radius + fDistance + src.Radius);
    Set(ptNewCenter, fNewRadius);
}

void Sphere::Extend(const Box3& src)
{
	Vec3	c = src.Center();
    Sphere s(c, c.Distance(src.min));
	Extend(s);
}

bool Sphere::Hit(const Ray& ray) const
{
	Vec3	c(Center - ray.start);
	Vec3	dir(ray.direction);
	float		dsq;

	dsq = (dir.x * c.z - dir.z * c.y) * (dir.y * c.z - dir.z * c.y)
		+ (dir.z * c.x - dir.x * c.z) * (dir.z * c.x - dir.x * c.z)
		+ (dir.x * c.y - dir.y * c.x) * (dir.x * c.y - dir.y * c.x);
	return (dsq <= Radius * Radius);
}

/*!
 * @fn float Sphere::Hit(const Plane& plane) const
 * @param plane	plane to check
 *
 * @return signed number as follows
 * - 0 sphere intersects plane
 * - < 0 if sphere is on the side of the plane the normal points towards
 * - > 0 for the other side. Larger magnitudes indicate the sphere is further from the plane.
 *
 * @see Box3::Hit Model::Hit
 */
float Sphere::Hit(const Plane& plane) const
{
	float dist = plane.Distance(Center);	// distance to plane
	if (fabs(dist) < Radius)				// intersected?
		return 0.0f;
	return dist;
}

/*!
 * @fn bool Sphere::Hit(const Ray &ray, float& distance, Vec3* intersect) const
 * @param distance	where to store distance of ray from sphere 
 * @param intersect	where to store intersection point
 *
 * @returns \b true if ray intersected sphere, else \b false
 *
 * @see Box3::Hit
 */
bool Sphere::Hit(const Ray &ray, float& distance, Vec3* intersect) const
{
	Vec3	c(Center - ray.start);
	Vec3	dir(ray.direction);
	float	v, dsq;

	v = c.Dot(dir);
	dsq = c.Dot(c) - v * v;
	dsq = Radius * Radius - dsq;
	if (dsq < 0)
		return false;
	distance = (float) FastSqrt(dsq);
	if (intersect)
	{
		dir *= (v - distance);
		*intersect = ray.start + dir;
	}
	return true;
}

}	// end Vixen
