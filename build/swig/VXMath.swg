/*
 * SWIG interface file for Vec2, Vec3, Vec4, Quat, Col4, Color, Matrix
 */
%define MATH_DEFINE(CTYPE)
	%naturalvar CTYPE;
	%ignore CTYPE::operator-=;
	%ignore CTYPE::operator+=;
	%ignore CTYPE::operator*=;
	%ignore CTYPE::operator/=;
	%ignore	CTYPE::operator=;
	%ignore	CTYPE::operator!=;
	%ignore CTYPE::operator-();
	%rename(Add)		CTYPE::operator+;
	%rename(Sub)		CTYPE::operator-(const CTYPE&) const;
	%rename(Mul)		CTYPE::operator*;
	%rename(Div)		CTYPE::operator/;
	%rename(Equals)		CTYPE::operator==;

	%typemap(cscode) CTYPE %{
		public static CTYPE operator +(CTYPE arg1, CTYPE arg2)	{ return arg1.Add(arg2); }
		public static CTYPE operator -(CTYPE arg1, CTYPE arg2)	{ return arg1.Sub(arg2); }
		public static CTYPE operator *(CTYPE vec, float val)	{ return vec.Mul(val); }
		public static CTYPE operator /(CTYPE vec, float val)	{ return vec.Div(val); }
	%}
%enddef

%ignore Vixen::Dot2;
%ignore Vixen::Dot3;
%ignore Vixen::Dot4;

namespace Vixen
{
	%naturalvar Quat;
	%naturalvar Ray;
	%ignore operator const float*;
	%ignore operator float*;
	%ignore	VX_SMALL_NUMBER;
	%ignore	VX_EPSILON;
	%ignore	VX_INFINITY;
	%ignore	MIN;
	%ignore MAX;
	%ignore FastSqrt;
	%ignore Plane;
	%ignore	Color::operator*;
	%rename(AsInt)	Col4::operator uint32;
	%rename(AsInt)	Color::operator uint32;

	MATH_DEFINE(Vec2)
	MATH_DEFINE(Vec3)
	MATH_DEFINE(Vec4)
	MATH_DEFINE(Col4)

	%ignore Quat::Quat(const Vec3*);
	%ignore Col4::operator Color;
	%ignore Col4::operator[];
	%ignore	Quat::operator=;
	%ignore	Quat::operator*=;
	%ignore	Quat::operator!=;
	%ignore Quat::operator-();
	%rename(Mul)		Quat::operator*;
	%rename(Equals)		Quat::operator==;
	%csmethodmodifiers	Quat::Normalize "new public"

	%typemap(cscode) Quat %{
		public static Quat operator*(Quat vec1, Quat vec2)
		{
			Quat q = new Quat();
			q.Mul(vec1, vec2);
			return q;
		}
		%}

	%naturalvar Color;
	%ignore Color::operator Col4;
	%ignore Color::operator[];
	%ignore Color::operator-=;
	%ignore Color::operator+=;
	%ignore Color::operator*=;
	%ignore Color::operator/=;
	%ignore	Color::operator=;
	%ignore	Color::operator!=;
	%ignore Color::operator-();
	%ignore Color::operator*;
	%rename(Add)		Color::operator+;
	%rename(Sub)		Color::operator-(const Color&) const;
	%rename(Equals)		Color::operator==;

	%typemap(cscode) Color %{
		public static Color operator +(Color arg1, Color arg2)	{ return arg1.Add(arg2); }
		public static Color operator -(Color arg1, Color arg2)	{ return arg1.Sub(arg2); }
	%}

	%ignore	Matrix::operator*=;
	%ignore	Matrix::RotationMatrix(const Vec3*);	
	%ignore	Matrix::GetMatrix;
	%ignore Matrix::operator=;
	%ignore Matrix::operator-=;
	%ignore Matrix::operator+=;
	%ignore Matrix::operator*=;
	%ignore Matrix::operator!=;
	%rename(Equals)		CTYPE::operator==;
}

#ifdef SWIGCSHARP
%csattributes Vixen::Vec2::LengthSquared "[System.ComponentModel.Browsable(false)]"
%csattributes Vixen::Vec3::LengthSquared "[System.ComponentModel.Browsable(false)]"
%csattributes Vixen::Vec4::LengthSquared "[System.ComponentModel.Browsable(false)]"
#endif 

ATTR_READONLY(Vixen::Vec2, float, Length, Length);
ATTR_READONLY(Vixen::Vec2, float, LengthSquared, LengthSquared);
ATTR_READONLY(Vixen::Vec3, float, Length, Length);
ATTR_READONLY(Vixen::Vec3, float, LengthSquared, LengthSquared);
ATTR_READONLY(Vixen::Vec4, float, Length, Length);
ATTR_READONLY(Vixen::Vec4, float, LengthSquared, LengthSquared);

%include "base/vxmath.h"
%include "base/vxcol.h"
%include "base/vxmatrix.h"