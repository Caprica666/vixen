/*!
 * @file vxmath.h
 * @brief Geometry and math classes for 3D graphics.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxmatrix.h vxcol.h
 */
#pragma once

#include <float.h>

#ifndef PI
#define PI 3.14159265358979f
#endif

#define FastSqrt sqrt

namespace Vixen
{

#define VX_SMALL_NUMBER	FLT_EPSILON
#define VX_EPSILON		FLT_EPSILON
#define VX_INFINITY		FLT_MAX

class Matrix;
class Messenger;

/*!
 * @fn float Dot2(const float* v1, const float* v2)
 *
 * Computes the dot product of two single-precision floating point
 * 2D vectors (requires 2 floats for X Y).
 * dot product = ( v1[0] * v2[0] + v1[1] * v2[1] )
 */
float Dot2(const float* floatArray1, const float* floatArray2);

/*!
 * @fn float Dot3(const float*, const float*)
 *
 * Computes the dot product of two single-precision floating point
 * 3D vectors (requires 3 floats for X Y Z).
 * dot product = ( v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2] )
 */
float Dot3(const float* floatArray1, const float* floatArray2);

/*!
 * @fn float Dot4(const float*, const float*)
 *
 * Computes the dot product of two single-precision floating point
 * 4D vectors (requires 4 floats for X Y Z W).
 * dot product = ( v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2] + v1[3] * v2[3} )
 */
float Dot4(const float* floatArray1, const float* floatArray2);

/*!
 * @class Vec2
 *
 * @brief Describes a 2D single-precision floating point vector.
 *
 * @ingroup vixen
 * @see Vec3
 */
class Vec2
{
	friend float Dot2(const float* floatArray1, const float* floatArray2);

public:
	float x;	//!< X component of vector.
	float y;	//!< Y component of vector.

//! @name Constructors
//@{
	Vec2();							//!< Constructor which does not initialize.
	Vec2(float x, float y);			//!< Construct vector from X and Y values.
	Vec2(const float* floatArray);	//!< Construct vector from array.
	Vec2(const Vec2&);				//!< Copy input vector into this one.
	void	Set(float x, float y);		//!< Set X, Y components of vector.
//@}

//! @name Operators
//@{
	Vec2&	operator=(const Vec2&);			//!< Copy input vector into this one.
	bool	operator==(const Vec2&) const;	//!< Test if vectors are equal.
	bool	operator!=(const Vec2&) const;	//!< Test if vectors are not equal.
	Vec2	operator-() const;				//!< Return negative of this vector.
	Vec2	operator+(const Vec2&) const;	//!< Compute sum of vectors.
	Vec2	operator-(const Vec2&) const;	//!< Compute difference between vectors.
	Vec2&	operator+=(const Vec2&);		//!< Add input vector to this vector.
	Vec2&	operator-=(const Vec2&);		//!< Subtract input vector from this one.
	Vec2	operator*(float) const;			//!< Return vector multiplied by constant.
	Vec2	operator/(float) const;			//!< Return vector divided by constant.
	Vec2&	operator*=(float);				//!< Multiply this vector by constant.
	Vec2&	operator/=(float);				//!< Divide this vector by constant.
	operator const float*() const	{ return &x; }
	operator float*()				{ return &x; }
//@}
	
//! @name Calculations
//@{
	Vec2&	Negate();					//!< Negate components of this vector.
	float	Length() const;				//!< Compute length of vector.
	float	LengthSquared() const;		//!< Compute squared length of vector.
	float	Normalize();				//!< Normalize this vector (make length == 1).
	float	Distance(const Vec2&) const;	//!< Compute distance between vectors.
	float	DistanceSquared(const Vec2& v) const;	//!< Computer squared distance between vectors.
	float	Dot(const Vec2&) const;	//!< Compute dot product of two vectors.
	float	Dot(const float* floatArray) const;	//!< Compute dot product of two vectors.
//@}
};

DebugOut& operator<<(DebugOut&, const Vec2&);


/*!
 * @class Vec3
 *
 * @brief Describes a 3D single-precision floating point vector.
 *
 * @ingroup vixen
 * @see Vec4 Vec2
 */
class Vec3
{
	friend class Matrix;
	friend float Dot3(const float* floatArray1, const float* floatArray2);

public:
	float x;	//!< X component of vector.
	float y;	//!< Y component of vector.
	float z;	//!< Z component of vector.

//! @name Constructors
//@{
	Vec3();								//!< Constructor which zeros X,Y,Z.
	Vec3(float x, float y, float z);	//!< Construct vector from X,Y,Z values.
	Vec3(const float* floatArray);		//!< Construct vector from array.
	Vec3(const Vec3&);					//!< Copy input vector into this one.
	void	Set(float x, float y, float z);	//!< Set X, Y, Z components of vector.
//@}

//! @name Operators
//@{
	Vec3&	operator=(const Vec3&);			//!< Copy input vector into this one.
	bool	operator==(const Vec3&) const;	//!< Test if vectors are equal.
	bool	operator!=(const Vec3&) const;	//!< Test if vectors are not equal.
	Vec3	operator-() const;				//!< Return negative of this vector.
	Vec3	operator-(const Vec3&) const;	//!< Compute difference between vectors.
	Vec3&	operator-=(const Vec3&);		//!< Subtract input vector from this one.
	Vec3	operator+(const Vec3&) const;	//!< Compute sum of vectors.
	Vec3&	operator+=(const Vec3&);		//!< Add input vector to this vector.
	Vec3	operator*(float) const;			//!< Return vector multiplied by constant.
	Vec3&	operator*=(float);				//!< Multiply this vector by constant.
	Vec3	operator*(const Vec3&) const;	//!< Multiply two vectors.
	Vec3&	operator*=(const Vec3&);		//!< Multiply two vectors.
	Vec3&	operator*=(const Matrix&);		//!< Multiply this vector by a matrix.
	Vec3	operator/(float) const;			//!< Return vector divided by constant.
	Vec3&	operator/=(float);				//!< Divide this vector by constant.
	operator const float*() const	{ return &x; }
	operator float*()				{ return &x; }
//@}
	
//! @name Calculations
//@{
	bool  	IsEmpty() const;
	Vec3	Cross(const Vec3&) const;	//!< Compute cross product of two vectors.
	Vec3&	Negate();					//!< Negate components of this vector.
	float	Length() const;				//!< Compute length of vector.
	float	LengthSquared() const;		//!< Compute squared length of vector.
	float	Normalize();				//!< Normalize this vector (make length == 1).
	float	Dot(const Vec3&) const;		//!< Compute dot product of two vectors.
	float	Dot(const float* floatArray) const;	//!< Compute dot product of two vectors.

//! Compute distance between vectors.
	float	Distance(const Vec3&) const;
//! Computer squared distance between vectors.
	float	DistanceSquared(const Vec3& v) const;
//! Multiply point by matrix.
	void	Transform(const Matrix&, const Vec3&);
//! Multiply vector by matrix.
	void	TransformVector(const Matrix&, const Vec3&);
//@}
};

DebugOut& operator<<(DebugOut&, const Vec3&);



/*!
 * @class Vec4
 *
 * @brief Describes a 3D single-precision floating point vector
 * in homogeneous coordinates.
 *
 * @see Vec3 Vec2 Quat
 */
class Vec4
{
	friend class Matrix;
	friend float Dot4(const float* floatArray1, const float* floatArray2, const float* floatArray3);

public:
	float x;	//!< X component of vector.
	float y;	//!< Y component of vector.
	float z;	//!< Z component of vector.
	float w;	//!< W component of vector.

//! @name Constructors
//@{
	Vec4();							//!< Constructor which zeros X,Y,Z.W.
	Vec4(const float* floatArray);	//!< Construct vector from array.
	Vec4(const Vec3&);				//!< Copy input vector into this one.
	Vec4(const Vec4&);				//!< Copy input vector into this one.
	Vec4(float x, float y, float z, float w);		//!< Construct vector from X,Y,Z,W values.
	void   Set(float x, float y, float z, float w);	//!< Set X, Y, Z, W components of vector.
//@}

//! @name Operators
//@{
	Vec4&	operator=(const Vec4&);			//!< Copy input vector into this one.
	bool	operator==(const Vec4&) const;	//!< Test if vectors are equal.
	bool	operator!=(const Vec4&) const;	//!< Test if vectors are not equal.
	Vec4	operator-() const;				//!< Return negative of this vector.
	Vec4	operator-(const Vec4&) const;	//!< Compute difference between vectors.
	Vec4&	operator-=(const Vec4&);		//!< Subtract input vector from this one.
	Vec4	operator+(const Vec4&) const;	//!< Compute sum of vectors.
	Vec4&	operator+=(const Vec4&);		//!< Add input vector to this vector.
	Vec4	operator*(float) const;			//!< Return vector multiplied by constant.
	Vec4&	operator*=(float);				//!< Multiply this vector by constant.
	Vec4&	operator*=(const Vec4&);		//!< Multiply two vectors.
	Vec4	operator/(float) const;			//!< Return vector divided by constant.
	Vec4&	operator/=(float);				//!< Divide this vector by constant.
	operator const float*() const	{ return &x; }
	operator float*()				{ return &x; }
//@}
	
//! @name Calculations
//@{
	bool  	IsEmpty() const;
	Vec4&	Negate();					//!< Negate components of this vector.
	float	Length() const;				//!< Compute length of vector.
	float	LengthSquared() const;		//!< Compute squared length of vector.
	float	Normalize();				//!< Normalize this vector (make length == 1).
	float	Dot(const Vec4&) const;		//!< Compute dot product of two vectors.
	float	Dot(const float* floatArray) const;	//!< Compute dot product of two vectors.

//! Compute distance between vectors.
	float	Distance(const Vec4&) const;
//! Computer squared distance between vectors.
	float	DistanceSquared(const Vec4& v) const;
//! Multiply vector by matrix.
	void	Transform(const Matrix&, const Vec4&);
//@}
};

DebugOut& operator<<(DebugOut&, const Vec4&);

/*!
 * @class Quat
 * @brief Quaternion class used to represent a rotation axis and angle.
 *
 * Given a unit axis [Ax Ay Az] and an \b angle about this axis,
 * the quaternion is
 * @code
 *	x = Ax * sin(angle / 2)
 *	y = Ay * sin(angle / 2)
 *	z = Az * sin(angle / 2)  
 *  w = cos(angle / 2)
 * @endcode
 *
 * @ingroup vixen
 * @see Vec4 Matrix
 */
class Quat : public Vec4
{
public:
//!	Initialize all components to zero.
	Quat() : Vec4() { }
//! Construct quaternion from X,Y,Z,W values.
	Quat(float X, float Y, float Z, float W) : Vec4(X, Y, Z, W) { };
//! Construct quaternion from axis and angle.
	Quat(const Vec3& axis, float ang);
//! Construct quaternion from rotation components of matrix.
	Quat(const Matrix& trans);
//! Copy vector into this quaternion.
	Quat(const Vec4& q) : Vec4(q) { };
//! Construction quaternion from angle between two vectors.
	Quat(const Vec3& v1, const Vec3& v2);
//! Construct quaternion from rotation vectors.
	Quat(const Vec3* INPUT);
//! True if quaternion is empty (0,0,0,1)
	bool	IsEmpty() const;

//! Get Euler angles from quaternion.
	void		GetEulerAngles(Vec3& euler_angles) const;
//! Set quaternion from axis and angle.
	void		Set(const Vec3& axis, float angle);
//! Set quaternion from rotation vectors.
	void		Set(const Vec3* INPUT);
//! Set quaternion from matrix.
	void		Set(const Matrix& mtx);
//! Set quaternion from angle between two vectors.
	void		Set(const Vec3& v1, const Vec3& v2);
//! Set quaternion as x,y,z,w
	void		Set(float x, float y, float z, float w)	{ Vec4::Set(x, y, z, w); }
//!	Multiply the input quaternions and store the result in this quaternion.
	void		Mul(const Quat& qL, const Quat& qR);
//! Multiply this quaternion by another.
	Quat&		operator*=(const Quat&);
//! Scale this quaternion by a scalar.
	Quat&		operator*=(float);
//! Perform spherical interpolation between two input quaternions.
	void		Slerp(const Quat& p, const Quat& q, float t);
//! Conjugate this quaternion.
	void		Conjugate();
//! Invert this quaternion.
	void		Invert();
//! Normalize this quaternion.
	void		Normalize();
	void		Squad(const Quat& p, const Quat& a, const Quat& b, const Quat& q, float t);
};


/*!
 * @class Ray
 *
 * @brief 3D line segment with a direction and a length.
 *
 * @ingroup vixen
 * @see VXLine
 */
class Ray
{
public:
	Vec3	start;		//!< starting point of line.
	Vec3	end;		//!< ending point of line
	Vec3	direction;	//!< unit direction of ray.
	float	length;		//!< length of ray.

//! Initialize all components to zero.
	Ray();

//! Make ray from starting point, direction and length.
	Ray(const Vec3 &start, const Vec3 &direction, float length);

//! Make ray from starting and ending points.
	Ray(const Vec3 &start, const Vec3 &end);

//! Initialize ray from endpoints
	void	Set(const Vec3& start, const Vec3& end);

//! Copy one ray into another.
	Ray&	operator=(const Ray& ray);

//! Compare two rays
	bool	operator==(const Ray& ray);
	bool	operator!=(const Ray& ray);
};

/*!
 * @class Plane
 * @brief 3D plane expressed in terms of its normal vector.
 *
 * @code
 *	x = X component of normal to plane
 *	y = Y component of normal to plane
 *	z = Z component of normal to plane
 *	w = distance of plane from origin (along normal)
 * @endcode
 *
 * @ingroup vixen
 * @see Vec4
 */
class Plane : public Vec4
{
public:
//! Initialize plane from normal vector and distance from origin.
	Plane(float nx, float ny, float nz, float dist)
	: Vec4(nx, ny, nz, dist) { }
//! Initialize plane from normal and distance values in array.
	Plane(float v[]) : Vec4(v) { };
//!	Initialize plane from 4D vector.
	Plane(const Vec4& v) : Vec4(v) { };
//! Initialize all components to zero.
	Plane() : Vec4() { };

//! Return normal to plane
	const Vec3& Normal() const;

//! Initialize plane from normal and position.
	void	Set(const Vec3& pos, const Vec3& normal);
//!	Compute the distance between this plane and the input vector.
	float	Distance(const Vec3&) const;
//! Determine if two planes intersect
	bool	Intersect(const Plane& src) const;
};


/*!
 * @fn Messenger& operator<<(Messenger& disp, const Vec2& v)
 * @param disp	messenger to output vector to
 * @param v		vector to write
 *
 * Outputs a 2D vector to the messenger.
 * The X and Y components are written as 32 bit floating point numbers.
 */
extern Messenger& operator<<(Messenger& d, const Vec2& v);
extern Messenger& operator>>(Messenger& d, Vec2& v);

/*!
 * @fn Messenger& operator<<(Messenger& disp, const Vec3& v)
 * @param disp	messenger to output vector to
 * @param v		vector to write
 *
 * Outputs a 3D vector to a messenger. The X, Y and Z components are
 * written as 32 bit floating point numbers.
 */
extern Messenger& operator<<(Messenger& d, const Vec3& v);
extern Messenger& operator>>(Messenger& d, Vec3& v);

/*!
 * @fn Messenger& operator<<(Messenger& disp, const Vec4& v)
 * @param disp	messenger to output vector to
 * @param v		vector to write
 *
 * Outputs a 3D homogeneous vector to a messenger.
 * The X, Y and Z components are written as 32 bit floating point numbers.
 */
extern Messenger& operator<<(Messenger& d, const Vec4& v);
extern Messenger& operator>>(Messenger& d, Vec4& v);


} // end Vixen
