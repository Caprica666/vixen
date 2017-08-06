/*
 * SWIG interface file for Box2, Box3
 */
namespace Vixen
{
	 %naturalvar Box3;
	 %naturalvar Box2;
	 %naturalvar Sphere;

	 %ignore Box3::Hit;
	 %ignore Box3::Extend(const Vec3*, int n = 1);
	 %ignore Sphere::Hit;

	 %rename(Assign)	Box2::operator=;
	 %rename(Assign)	Box3::operator=;
	 %rename(Mul)		Box3::operator*=;
	 %rename(Div)		Box3::operator/=;
	 %rename(Equal)		Box3::operator==;
	 %rename(NotEqual)	Box3::operator!=;
	 %rename(Assign)	Sphere::operator=;
	 %rename(Mul)		Sphere::operator*=;
	 %rename(Div)		Sphere::operator/=;
	 %rename(Equal)		Sphere::operator==;
	 %rename(NotEqual)	Sphere::operator!=;
}

ATTR_READONLY(Vixen::Box2, float, Height, Height);
ATTR_READONLY(Vixen::Box2, float, Width, Width);
ATTR_READONLY(Vixen::Box2, Vixen::Vec2, Center, Center);

ATTR_READONLY(Vixen::Box3, float, Height, Height);
ATTR_READONLY(Vixen::Box3, float, Width, Width);
ATTR_READONLY(Vixen::Box3, float, Depth, Depth);
ATTR_READONLY(Vixen::Box3, Vixen::Vec3, Center, Center);

 %include "base/vxbox.h"
 %include "base/vxsphere.h"


