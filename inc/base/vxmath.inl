// RAM_XX

/****
 *
 * 
 * __forceinlines for Vec2 Vec2 Vec4 Quat
 *
 *
 ****/
#pragma once

namespace Vixen {

/*****
 *
 * 	Dot2(float*, float*)
 * 	Dot3(float*, float*)
 * 	Dot4(float*, float*)
 *
 * Description:
 *	2D, 3D and 4D vector dot products. The floating point
 *	input arguments are assumed to be ordered X, Y, Z
 *
 *****/
FORCE_INLINE float 
Dot2(const float * const d1, const float * const d2)
{
	return d1[0]*d2[0]+d1[1]*d2[1];
}

FORCE_INLINE float 
Dot3(const float *d1, const float *d2)
{
	return d1[0]*d2[0]+d1[1]*d2[1]+d1[2]*d2[2];
}

FORCE_INLINE float 
Dot4(const float *d1, const float *d2)
{
	return d1[0]*d2[0]+d1[1]*d2[1]+d1[2]*d2[2]+d1[3]*d2[3];
}

FORCE_INLINE Vec2::Vec2(float d0, float d1)
: x(d0), y(d1)
{
}

FORCE_INLINE Vec2::Vec2(const float* v) 
{
	x = v[0];
	y = v[1];
}

FORCE_INLINE Vec2::Vec2(const Vec2 &v)
{
	x = v.x;
	y = v.y;
}

FORCE_INLINE Vec2::Vec2 ()
{ }

FORCE_INLINE Vec2& Vec2::operator=(const Vec2& rhs)
{
	if (this != &rhs)
	{
		x = rhs.x;
		y = rhs.y;
	}
	return *this;
}

FORCE_INLINE void Vec2::Set(float xx, float yy)
{
	x = xx;
	y = yy;
}

/*!
 * @fn Vec2& Vec2::Negate()
 *
 * Replaces each component with its negative:
 *	x = -x, y = -y
 *
 * @see Vec2::operator-
 */
FORCE_INLINE Vec2& Vec2::Negate()
{
	x = -x;
	y = -y;
	return *this;
}

/*!
 * @fn Vec2 Vec2::operator-() const
 *
 * Creates a new vector with negated components -x, -y
 *
 * @see Vec2::Negate
 */
FORCE_INLINE Vec2 Vec2::operator-() const
{
	return Vec2(-x, -y);
}

/*!
 * @fn Vec2 Vec2::operator+(const Vec2& v) const
 *
 * Creates a new vector by summing individual components:
 *	x + v.x, y + v.y
 *
 * @see Vec2::operator+= Vec2::operator-
 */
FORCE_INLINE Vec2 Vec2::operator+(const Vec2& v) const
{
	return Vec2(x + v.x, y + v.y);
}

/*!
 * @fn Vec2& Vec2::operator+=(const Vec2& v)
 *
 * Adds to this vector by summing components:
 *	x += v.x, y += v.y
 *
 * @see Vec2::operator+ Vec2::operator-=
 */
FORCE_INLINE Vec2& Vec2::operator+=(const Vec2& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

/*!
 * @fn Vec2& Vec2::operator-=(const Vec2& v)
 *
 * Subtracts input vector from this one by subtracting components:
 *	x -= v.x, y -= v.y
 *
 * @see Vec2::operator+ Vec2::operator-=
 */
FORCE_INLINE Vec2& Vec2::operator-=(const Vec2& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

/*!
 * @fn Vec2 Vec2::operator-(const Vec2& v) const
 *
 * Creates a new vector by subtracting individual components:
 *	x - v.x, y - v.y
 *
 * @see Vec2::operator-= Vec2::operator+
 */
FORCE_INLINE Vec2 Vec2::operator-(const Vec2 &v) const
{
	return Vec2(x - v.x, y - v.y);
}

/*!
 * @fn Vec2 Vec2::operator*(float f) const
 *
 * Creates a new vector by multiplying with a constant:
 *	x * f, y * f
 *
 * @see Vec2::operator*= Vec2::operator/
 */
FORCE_INLINE  Vec2 Vec2::operator*(float f) const
{
	return Vec2(x * f, y * f);
}

/*!
 * @fn Vec2 Vec2::operator/(float f) const
 *
 * Creates a new vector by dividing with a constant:
 *	x / f, y / f
 **
 * @see Vec2::operator/= Vec2::operator/
 */
FORCE_INLINE Vec2 Vec2::operator/(float f) const
{
	VX_ASSERT( 0 != f );
	return Vec2(x / f, y / f);
}

/*!
 * @fn Vec2 Vec2::operator*=(float f)
 *
 * Multiplies each component by a constant
 *	x *= f, y *= f
 *
 * @see Vec2::operator* Vec2::operator/=
 */
FORCE_INLINE Vec2& Vec2::operator*=(float f)
{
	x *= f;
	y *= f;
	return *this;
}

/*!
 * @fn Vec2 Vec2::operator/=(float f)
 *
 * Divides each component by a constant
 *	x /= f, y /= f
 *
 * @see Vec2::operator* Vec2::operator/=
 */
FORCE_INLINE Vec2& Vec2::operator/=(float f)
{
	VX_ASSERT(0 != f);
	return operator*=(1.0f / f);
}

/*!
 * @fn bool Vec2::operator==(const Vec2& v) const
 *
 * Vectors are equal if the difference between
 * corresponding components of the vectors are
 * within a small floating point tolerance.
 *
 * @returns \b true if vectors very close, else \b false
 *
 * @see Vec2::operator!=
 */
FORCE_INLINE bool Vec2::operator==(const Vec2& v) const
{
	return ( fabs(x - v.x) < VX_SMALL_NUMBER &&
			 fabs(y - v.y) < VX_SMALL_NUMBER );
}

/*!
 * @fn bool Vec2::operator!=(const Vec2& v) const
 *
 * Vectors are not equal if the difference between
 * corresponding components of the vectors are
 * greater than a small floating point tolerance.
 *
 * @returns \b true if vectors are different, else \b false
 *
 * @see Vec2::operator==
 */
FORCE_INLINE bool Vec2::operator!=(const Vec2& v) const
{
	return !(*this == v);
}

/*!
 * @fn float Vec2::Dot(const Vec2& v) const
 *
 * The dot product is calculated by multiplying
 * corresponding components and adding them together:
 * dot product = x * v.x + y * v.y
 *
 * @see Dot2
 */
FORCE_INLINE float Vec2::Dot(const Vec2 &v) const
{
	return Dot2((float*) &v.x, (float*) &x);
}

/*!
 * @fn float Vec2::Dot(const float* v) const
 *
 * The dot product is calculated by multiplying
 * corresponding components and adding them together:
 * dot product = x * v[0] + y * v[1]
 *
 * @see Dot2
 */
FORCE_INLINE float Vec2::Dot(const float* v) const
{
	return Dot2((float*) &x, v);
}

/*!
 * @fn float Vec2::Length() const
 *
 * The length of a vector is calculated by squaring
 * all components and taking the square root of the result:
 *	length = sqrt( v.x^2 + v.y^2 )
 *
 * @see Vec2::Normalize Vec2::Distance Vec2::LengthSquared
 */
FORCE_INLINE float Vec2:: Length() const
{
	return (float) sqrt( LengthSquared() );
}

/*!
 * @fn float Vec2::LengthSquared() const
 *
 * The squared length of a vector is calculated by squaring all components:
 *	lengthsq = v.x^2 + v.y^2 + v.z^2
 *
 * @see Vec2::Normalize Vec2::Distance Vec2::Length
 */
FORCE_INLINE float  Vec2:: LengthSquared() const
{
	return Dot(*this);
}

/*!
 * @fn float Vec2::Normalize()
 *
 * Replaces this vector with a unit vector in the same direction.
 * This operation is somewhat expensive in that it computes the
 * length of the vector and divides each component by it which
 * involves computing a square root.
 *
 * @see Vec2::Length Vec2::LengthSquared
 */
FORCE_INLINE float Vec2:: Normalize()
{
	float l = Length();
	if (l > VX_SMALL_NUMBER)
	{
		x /= l;
		y /= l; 
	}
	else
	{
		x = 0.0f;
		y = 1.0f;
	}
	return l;
}

/*!
 *
 * @fn float Vec2::DistanceSquared(const Vec2& v) const
 *
 * The distance is calculated by squaring the components of
 * the difference vector:
 *	distance = (x - v.x) ^2 + (y - v.y) ^2 
 *
 * @see Vec2::Normalize Vec2::Distance Vec2::LengthSquared
 */
FORCE_INLINE float Vec2::DistanceSquared(const Vec2& v) const
{
	return (*this - v).LengthSquared();
}

/*!
 * @fn float Vec2::Distance(const Vec2& v) const
 *
 * The distance is calculated by computing the length of
 * the difference vector:
 *	distance = sqrt( (x - v.x) ^2 + (y - v.y) ^2
 *
 * @see Vec2::Normalize Vec2::DistanceSquared Vec2::Length
 */
FORCE_INLINE float Vec2::Distance(const Vec2& v) const
{
	return (*this - v).Length();
}

FORCE_INLINE DebugOut& operator<<(DebugOut& s, const Vec2& v)
{
    s << v.x << " " << v.y;
	return s;
}

FORCE_INLINE Vec3:: Vec3(float a, float b, float c) 
{
	x = a;
	y = b;
	z = c;
}

FORCE_INLINE Vec3:: Vec3(const float* v) 
: x(v[0]), y(v[1]), z(v[2])
{
}

FORCE_INLINE Vec3:: Vec3(const Vec3& v)
: x(v.x), y(v.y), z(v.z)
{
}

FORCE_INLINE Vec3::Vec3()
: x(0), y(0), z(0)
{
}

FORCE_INLINE Vec3& Vec3::operator=(const Vec3& rhs)
{
	if (this != &rhs)
	{
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
	}
	return *this;
}

FORCE_INLINE void Vec3::Set(float xx, float yy, float zz)
{
	x = xx;
	y = yy;
	z = zz;
}

/*!
 * @fn Vec3& Vec3::Negate()
 *
 * Replaces each component with its negative:
 *	x = -x, y = -y, z = -z
 *
 * @see Vec3::operator-
 */
FORCE_INLINE Vec3& Vec3::Negate()
{
	x = -x;
	y = -y;
	z = -z;
	return *this;
}

/*!
 * @fn Vec2 Vec3::operator-() const
 *
 * Creates a new vector with negated components -x, -y, -z
 *
 * @see Vec3::Negate
 */
FORCE_INLINE Vec3 Vec3::operator-() const
{
	return Vec3(-x, -y, -z);
}

/*!
 * @fn Vec3 Vec3::operator+(const Vec3& v) const
 *
 * Creates a new vector by summing individual components:
 *	x + v.x, y + v.y, z + v.z
 *
 * @see Vec3::operator+= Vec3::operator-
 */
FORCE_INLINE Vec3 Vec3::operator+(const Vec3& v) const
{
	return Vec3(x + v.x, y + v.y, z  +v.z);
}

/*!
 * @fn Vec3& Vec3::operator+=(const Vec3& v);
 *
 * Adds to this vector by summing components:
 *	x += v.x, y += v.y, z += v.z
 *
 * @see Vec3::operator+ Vec3::operator-=
 */
FORCE_INLINE Vec3& Vec3::operator+=(const Vec3 &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}


/*!
 * @fn Vec3 Vec3::operator-(const Vec3& v) const
 *
 * Creates a new vector by subtracting individual components:
 *	x - v.x, y - v.y, z - v.z
 *
 * @see Vec3::operator-= Vec3::operator+
 */
FORCE_INLINE Vec3 Vec3::operator-(const Vec3 &v) const
{
	return Vec3(x - v.x, y - v.y, z - v.z);
}

/*!
 * @fn Vec3& Vec3::operator-=(const Vec3& v)
 *
 * Subtracts input vector from this one by subtracting components:
 *	x -= v.x, y -= v.y, z -= v.z
 *
 * @see Vec3::operator+ Vec3::operator-=
 */
FORCE_INLINE Vec3& Vec3::operator-=(const Vec3& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

/*!
 * @fn Vec3 Vec3::operator*(float f) const
 *
 * Creates a new vector by multiplying with a constant:
 *	x * f, y * f, z * f
 *
 * @see Vec3::operator*= Vec3::operator/
 */
FORCE_INLINE Vec3 Vec3::operator*(float f) const
{
	return Vec3(x*f, y*f, z*f);
}

/*!
 * @fn Vec3 Vec3::operator/(float f) const
 *
 * Creates a new vector by dividing with a constant:
 *	x / f, y / f, z / f
 *
 * @see Vec3::operator/= Vec3::operator*
 */
FORCE_INLINE Vec3 Vec3::operator/(float f) const
{
	VX_ASSERT( 0 != f );
	return Vec3(x / f, y / f, z / f);
}

/*!
 * @fn Vec3& Vec3::operator*=(float f)
 *
 * Multiplies each component by a constant
 *	x *= f, y *= f, z *= f
 *
 * @see Vec3::operator* Vec3::operator/=
 */
FORCE_INLINE Vec3& Vec3::operator*=(float f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

/*!
 * @fn Vec3& Vec3::operator*=(const Vec3& v)
 *
 * Multiplies this vector by input vector
 *	x *= v.x, y *= v.y, z *= v.z
 */
FORCE_INLINE Vec3& Vec3::operator*=(const Vec3& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

/*!
 * @fn Vec3 Vec3::operator*(const Vec3& v) const
 *
 * Multiplies the vector components together:
 *	x * v.x, y * v.y, z *= v.z
 */
FORCE_INLINE Vec3 Vec3::operator*(const Vec3& v) const
{
	return Vec3( x * v.x, y * v.y, z * v.z );
}

/*!
 * @fn Vec3& Vec3::operator/=(float f)
 *
 * Divides each component by a constant
 *	x /= f, y /= f, z /= f
 *
 * @see Vec3::operator* Vec3::operator/=
 */
FORCE_INLINE Vec3& Vec3::operator/=(float f)
{
	VX_ASSERT( 0 != f );
	return operator*=(1.0f/f);
}


/*!
 * @fn bool Vec3::operator==(const Vec3& v) const
 *
 * Vectors are equal if the difference between
 * corresponding components of the vectors are
 * within a small floating point tolerance.
 *
 * @returns \b true if vectors very close, else \b false
 *
 * @see Vec3::operator!=
 */
FORCE_INLINE bool Vec3::operator==(const Vec3 &v) const
{
	return( fabs(x - v.x) < VX_SMALL_NUMBER &&
			fabs(y - v.y) < VX_SMALL_NUMBER &&
			fabs(z - v.z) < VX_SMALL_NUMBER );
}

/*!
 * @fn bool Vec3::operator!=(const Vec3& v) const
 *
 * Vectors are not equal if the difference between
 * corresponding components of the vectors are
 * greater than a small floating point tolerance.
 *
 * @returns \b true if vectors are different, else \b false
 *
 * @see Vec3::operator==
 */
FORCE_INLINE bool Vec3::operator!=(const Vec3 &v) const
{
	return !(*this == v);
}

/*!
 * @fn float Vec3::Dot(const Vec3& v) const
 *
 * The dot product is calculated by multiplying
 * corresponding components and adding them together:
 * dot product = x * v.x + y * v.y + z * v.z
 *
 * @see Dot3
 */
FORCE_INLINE float Vec3::Dot(const Vec3& v) const
{
	return Dot3((const float*) *this, (const float*) v);
}

/*!
 * @fn float Vec3::Dot(const float* v) const
 *
 * The dot product is calculated by multiplying
 * corresponding components and adding them together:
 * dot product = x * v[0] + y * v[1] + z * v[2]
 *
 * @see Dot3
 */
FORCE_INLINE float Vec3::Dot(const float *v) const
{
	return Dot3((float*) &x, v);
}

/*!
 * @fn float Vec3::Length() const
 *
 * The length of a vector is calculated by squaring
 * all components and taking the square root of the result:
 *	length = sqrt( v.x^2 + v.y^2 + v.z^2 )
 *
 * @see Vec3::Normalize Vec3::Distance Vec3::LengthSquared
 */
FORCE_INLINE float Vec3::Length() const
{
	return (float)sqrt( LengthSquared() );
}

/*!
 * @fn float Vec3::LengthSquared() const
 *
 * The squared length of a vector is calculated by squaring all components:
 *	lengthsq = v.x^2 + v.y^2 + v.z^2
 *
 * @see Vec3::Normalize Vec3::Distance Vec3::Length
 */
FORCE_INLINE float Vec3::LengthSquared() const
{
	return Dot3(*this, *this);
}

/*!
 * @fn float Vec3::Distance(const Vec3& v) const
 *
 * The distance is calculated by computing the length of
 * the difference vector:
 *	distance = sqrt( (x - v.x) ^2 + (y - v.y) ^2 + (z - v.z) ^2 )
 *
 * @see Vec3::Normalize Vec3::DistanceSquared Vec3::Length
 */
FORCE_INLINE float Vec3::Distance(const Vec3 &v) const
{
	return ((*this) - v ).Length();
}

/*!
 * @fn float Vec3::DistanceSquared(const Vec3& v) const
 *
 * The distance is calculated by squaring the components of
 * the difference vector:
 *	distance = (x - v.x) ^2 + (y - v.y) ^2 + (z - v.z) ^2
 *
 * @see Vec3::Normalize Vec3::Distance Vec3::LengthSquared
 */
FORCE_INLINE float Vec3::DistanceSquared(const Vec3 &v) const
{
	return ((*this) - v ).LengthSquared();
}

/*!
 * @fn float Vec3::Normalize()
 *
 * Replaces this vector with a unit vector in the same direction.
 * This operation is somewhat expensive in that it computes the
 * length of the vector and divides each component by it which
 * involves computing a square root.
 *
 * @see Vec3::Length Vec3::LengthSquared
 */
// RAM_XX
#if 1
FORCE_INLINE float Vec3::Normalize()
{
	float l = Length();
	if (l > VX_SMALL_NUMBER)
	{
		x /= l;
		y /= l; 
		z /= l; 
	}
	else
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
	return l;
}
#else
FORCE_INLINE float Vec3::Normalize()
{
	float   r = 0.0f;
	float l = Dot3(*this, *this);
	if (l > VX_SMALL_NUMBER)
	{
		_mm_store_ss( & r, _mm_rsqrt_ss( _mm_load_ss( & l ) ) );
		r *= ((3.0f - r * r * l) * 0.5f);
    
		x *= r;
		y *= r; 
		z *= r; 
	}
	else
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
	return r;
}
#endif

/*!
 * @fn Vec3 Vec3::Cross(const Vec3& v) const
 *
 * Computes the cross product of two vectors.
 *
 * @see VXVec3f::Dot
 */
FORCE_INLINE Vec3 Vec3::Cross(const Vec3 &v) const
{
	return Vec3(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
}

/*!
 * @fn Vec3::operator*=(const Matrix& mtx)
 * @param mtx 4x4 matrix to multiply vector by.
 *
 * Transforms the \b point by the given matrix.
 * The point is treated as a 1x3 row vector and is
 * post-multiplied by the 3x3 part of the 4x4 matrix.
 * Projection components of the matrix are ignored.
 *
 * @code
 *		|x y z|	| M00 M10 M20 | + | M03 |
 *				| M01 M11 M21 |	  | M13 |
 *				| M02 M12 M22 |	  | M23 |
 * @endcode
 *
 * @see Matrix::Transform Vec3::TransformVector Vec3::Transform
 */
FORCE_INLINE Vec3& Vec3::operator*=(const Matrix& mtx)
{
	Vec3 src(*this);

	mtx.Transform(src, *this);
	return *this;
}

/*!
 * @fn bool Vec3::IsEmpty() const
 *
 * @return \b true if the vector is initialized to zero, else \b false
 */
FORCE_INLINE bool Vec3::IsEmpty( void ) const
{
	return( (x == 0) && (y == 0) && (z == 0) );
}


FORCE_INLINE void Vec3::Transform(const Matrix& mtx, const Vec3& src) 
{
	mtx.Transform(src, *this);
}

FORCE_INLINE DebugOut& operator<<(DebugOut& s, const Vec3& v)
{
    s << v.x << " " << v.y << " " << v.z;
	return s;
}

FORCE_INLINE Vec4::Vec4(float d0, float d1, float d2, float d3)
: x(d0), y(d1), z(d2), w(d3)
{
}

FORCE_INLINE Vec4:: Vec4(const float* v)
: x(v[0]), y(v[1]), z(v[2]), w(v[3])
{
}

FORCE_INLINE Vec4:: Vec4(const Vec4& v)
: x(v.x), y(v.y), z(v.z), w(v.w)
{
}

FORCE_INLINE Vec4::Vec4(const Vec3& v)
: x(v.x), y(v.y), z(v.z), w(1.0f)
{
}

FORCE_INLINE Vec4::Vec4()
: x(0), y(0), z(0), w(0)
{
}

/*!
 * @fn bool Vec4::IsEmpty() const
 *
 * @return \b true if the vector is initialized to zero, else \b false
 */
FORCE_INLINE bool Vec4::IsEmpty() const
{
	return ((x == 0) && (y == 0) && (z == 0) && (w == 0));
}

FORCE_INLINE Vec4& Vec4::operator=(const Vec4& rhs)
{
	if (this != &rhs)
	{
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		w = rhs.w;
	}
	return *this;
}

FORCE_INLINE void  Vec4::Set(float xx, float yy, float zz, float ww)
{
	x = xx;
	y = yy;
	z = zz;
	w = ww;
}

/*!
 * @fn Vec4& Vec4::Negate()
 *
 * Replaces each component with its negative:
 *	x = -x, y = -y, z = -z, w = -w
 *
 * @see Vec3::operator-
 */
FORCE_INLINE Vec4& Vec4::Negate()
{
	x = -x;
	y = -y;
	z = -z;
	w = -w;
	return *this;
}

/*!
 * @fn Vec4 Vec4::operator-() const
 *
 * Creates a new vector with negated components -x, -y, -z, -w
 *
 * @see Vec4::Negate
 */
FORCE_INLINE Vec4 Vec4::operator-() const
{
	return Vec4(-x, -y, -z, -w);
}

/*!
 * @fn Vec4 Vec4::operator+(const Vec4& v) const
 *
 * Creates a new vector by summing individual components:
 *	x + v.x, y + v.y, z + v.z, w + v.w
 *
 * @see Vec4::operator+= Vec4::operator-
 */
FORCE_INLINE Vec4 Vec4::operator+(const Vec4& v) const
{
	return Vec4(x+v.x, y+v.y, z+v.z, w+v.w);
}

/*!
 * @fn Vec4& Vec4::operator+=(const Vec4& v)
 *
 * Adds to this vector by summing components:
 *	x += v.x, y += v.y, z += v.z, w += v.w
 *
 * @see Vec4::operator+ Vec4::operator-=
 */
FORCE_INLINE Vec4& Vec4::operator+=(const Vec4& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

/*!
 * @fn Vec4& Vec4::operator-=(const Vec4& v)
 *
 * Subtracts input vector from this one by subtracting components:
 *	x -= v.x, y -= v.y, z -= v.z, w -= v.w
 *
 * @see Vec4::operator+ Vec4::operator-=
 */
FORCE_INLINE Vec4& Vec4::operator-=(const Vec4& v)
{
	x -= v.x;
	y -= v.y; 
	z -= v.z;
	w -= v.w;
	return *this;
}

/*!
 * @fn Vec4 Vec4::operator-(const Vec4& v) const
 *
 * Creates a new vector by subtracting individual components:
 *	x - v.x, y - v.y, z - v.z, w - v.w
 *
 * @see Vec4::operator-= Vec4::operator+
 */
FORCE_INLINE Vec4 Vec4::operator-(const Vec4& v) const
{
	return Vec4(x-v.x, y-v.y, z-v.z, w-v.w);
}

/*!
 * @fn Vec4 Vec4::operator*(float f) const
 *
 * Creates a new vector by multiplying with a constant:
 *	x * f, y * f, z * f, w * f
 *
 * @see Vec4::operator*= Vec4::operator/
 */
FORCE_INLINE Vec4 Vec4::operator*(float f) const
{
	return Vec4(x*f, y*f, z*f, w*f);
}

/*!
 * @fn Vec4& Vec4::operator*=(const Vec4& v)
 *
 * Multiplies this vector by input vector
 *	x *= v.x, y *= v.y, z *= v.z, w = v.w
 */
FORCE_INLINE Vec4& Vec4::operator*=(const Vec4& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	w *= v.w;
	return *this;
}
/*!
 * @fn Vec4 Vec3::operator/(float f) const
 *
 * Creates a new vector by dividing with a constant:
 *	x / f, y / f, z / f, w / f
 *
 * @see Vec4::operator/= Vec4::operator*
 */
FORCE_INLINE Vec4 Vec4::operator/(float f) const
{
	VX_ASSERT( 0 != f );
	return operator*(1.0f / f);
}

/*!
 * @fn Vec4& Vec4::operator*=(float f)
 *
 * Multiplies each component by a constant
 *	x *= f, y *= f, z *= f, w *= f
 *
 * @see Vec4::operator* Vec4::operator/=
 */
FORCE_INLINE Vec4& Vec4::operator*=(float f)
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}

/*!
 * @fn Vec4& Vec4::operator/=(float f)
 *
 * Divides each component by a constant
 *	x /= f, y /= f, z /= f
 *
 * @see Vec4::operator* Vec4::operator/=
 */
FORCE_INLINE Vec4& Vec4::operator/=(float f)
{
	VX_ASSERT( 0 != f );
	return operator*=(1.0f/f);
}

/*!
 * @fn bool Vec4::operator==(const Vec4& v) const
 *
 * Vectors are equal if the difference between
 * corresponding components of the vectors are
 * within a small floating point tolerance.
 *
 * @returns \b true if vectors very close, else \b false
 *
 * @see Vec3::operator!=
 */
FORCE_INLINE bool Vec4::operator==(const Vec4& v) const
{
	return( fabs(x - v.x) < VX_SMALL_NUMBER &&
			fabs(y - v.y) < VX_SMALL_NUMBER &&
			fabs(z - v.z) < VX_SMALL_NUMBER &&
			fabs(w - v.w) < VX_SMALL_NUMBER );
}

/*!
 * @fn bool Vec4::operator!=(const Vec4& v) const
 *
 * Vectors are not equal if the difference between
 * corresponding components of the vectors are
 * greater than a small floating point tolerance.
 *
 * @returns \b true if vectors are different, else \b false
 *
 * @see VXVec4f::operator==
 */
FORCE_INLINE bool Vec4::operator!=(const Vec4& v) const
{
	return !(*this == v);
}

/*!
 * @fn float Vec3::Dot(const float* v) const
 *
 * The dot product is calculated by multiplying
 * corresponding components and adding them together:
 * dot product = x * v[0] + y * v[1] + z * v[2] + w * v[3]
 *
 * @see Dot4
 */
FORCE_INLINE float Vec4::Dot(const Vec4& v) const
{
	return Dot4(*this, v);
}

FORCE_INLINE float Vec4::Dot(const float* v) const
{
	return Dot4((float*) &x, v);
}

/*!
 * @fn float Vec4::Length() const
 *
 * The length of a vector is calculated by squaring
 * all components and taking the square root of the result:
 *	length = sqrt( v.x^2 + v.y^2 + v.z^2  + v.w^2 )
 *
 * @see Vec4::Normalize Vec4::Distance Vec4::LengthSquared
 */
FORCE_INLINE float Vec4:: Length() const
{
	return (float)sqrt( LengthSquared() );
}

/*!
 * @fn float Vec4::LengthSquared() const
 *
 * The squared length of a vector is calculated by squaring all components:
 *	lengthsq = v.x^2 + v.y^2 + v.z^2 + v.w^2
 *
 * @see Vec3::Normalize Vec3::Distance Vec3::Length
 */
FORCE_INLINE float Vec4:: LengthSquared() const
{
	return Dot4(*this, *this);
}

/*!
 * @fn float Vec4::Distance(const Vec4& v) const
 *
 * The distance is calculated by computing the length of
 * the difference vector:
 *	distance = sqrt( (x - v.x) ^2 + (y - v.y) ^2 + (z - v.z) ^2 + (w - v.w) ^2)
 *
 * @see Vec4::Normalize Vec4::DistanceSquared Vec4::Length
 */
FORCE_INLINE float Vec4::Distance(const Vec4& v) const
{
	return (*this - v).Length();
}

/*!
 * @fn float Vec4::DistanceSquared(const Vec4& v) const
 *
 * The distance is calculated by squaring the components of
 * the difference vector:
 *	distance = (x - v.x) ^2 + (y - v.y) ^2 + (z - v.z) ^2 + (w - v.w) ^2
 *
 * @see Vec4::Normalize Vec4::Distance Vec4::LengthSquared
 */
FORCE_INLINE float Vec4::DistanceSquared(const Vec4& v) const
{
	return (*this - v).LengthSquared();
}

/*!
 * @fn float Vec4::Normalize()
 *
 * Replaces this vector with a unit vector in the same direction.
 * This operation is somewhat expensive in that it computes the
 * length of the vector and divides each component by it which
 * involves computing a square root.
 *
 * @see Vec4::Length Vec4::LengthSquared
 */
FORCE_INLINE float Vec4::Normalize()
{
	float l = Length();
	if (l > VX_SMALL_NUMBER) {
		operator*=(1.0f/l);
	} else {
		x = 0.0f; 
		y = 1.0f; 
		z = 0.0f;
		w = 1.0f;
	}
	return l;
}


/*!
 * @fn void Vec4::Transform(const Matrix& mtx, const Vec4& src)
 * @param mtx	4x4 matrix to multiply source vector by.
 * @param src	source vector.

 *
 * Transforms the \b point in the source vector by the given matrix.
 * The point is treated as a 1x4 row vector and is
 * post-multiplied by the 4x4 matrix.
 *
 * @code
 *		|x y z w| | M00 M10 M20 M30 |
 *			  	  | M01 M11 M21 M31 |
 *				  | M02 M12 M22 M32 |
 *				  | M03 M13 M23 M33 |
 * @endcode
 *
 * @see Matrix::Transform Vec3::TransformVector Vec4::operator*=
 */
FORCE_INLINE void Vec4::Transform(const Matrix &mtx, const Vec4& src)
{
	mtx.Transform(src, *this);
}

FORCE_INLINE void Quat::Set(const Vec3* vecs)
{
	Matrix mtx;

	mtx.RotationMatrix(vecs);
	Set(mtx);
}

FORCE_INLINE Quat& Quat::operator*=(float val)
{
	x *= val;
	y *= val;
	z *= val;
	w *= val;
	return *this;
}

FORCE_INLINE DebugOut& operator<<(DebugOut& s, const Vec4& v)
{
    s << v.x << " " << v.y << " " << v.z << " " << v.w;
	return s;
}

FORCE_INLINE Ray::Ray()
{
	length = 0;
}


FORCE_INLINE Ray::Ray(const Vec3& s, const Vec3& e)
: start(s), end(e)
{
// Since we already paid the price of finding direction length.
// we can normalize the direction vector.
	direction = end - start;
	length = direction.Length();
	VX_ASSERT( length > VX_SMALL_NUMBER );
	direction *= 1.0f / length;
}	

FORCE_INLINE Ray::Ray(const Vec3& s, const Vec3& d, float l)
: direction(d), length(l)
{
// We assume user supply a normalized vector d.
	float tmp = d.LengthSquared() - 1.0f;
	VX_ASSERT( fabs(d.LengthSquared()-1.0f) < VX_SMALL_NUMBER);
	start = s;
	end = start + direction * l;
}

FORCE_INLINE Ray& Ray::operator=(const Ray& r)
{
	start = r.start;
	end  = r.end;
	direction = r.direction;
	length = r.length;
	return *this;
}

FORCE_INLINE void Ray::Set(const Vec3& s, const Vec3& e)
{
	start = s;
	end  = e;
	direction = e - s;
	length = direction.Length();
	VX_ASSERT(length > VX_SMALL_NUMBER);
	direction *= 1.0f / length;
}

FORCE_INLINE bool Ray::operator==(const Ray& r)
{
	return (start == r.start) && (end == r.end);
}

FORCE_INLINE bool Ray::operator!=(const Ray& r)
{
	return (start != r.start) || (end != r.end);
}

FORCE_INLINE const Vec3& Plane::Normal() const
{
	#pragma warning(disable:473)
	return (const Vec3&) *this;
}

FORCE_INLINE void Plane::Set(const Vec3& p, const Vec3& n)
{
	x = n.x;
	y = n.y;
	z = n.z;
	w = -n.Dot(p);
}

FORCE_INLINE float Plane::Distance(const Vec3& v) const
{
	return Dot3(*this, v) + w;
}

FORCE_INLINE bool Plane::Intersect(const Plane& src) const
{
    Vec3 cross = Normal().Cross(src.Normal());
    return cross.LengthSquared() > VX_EPSILON;
}

} // end Vixen
