#include "vixen.h"

namespace Vixen {

/*!
 * @fn Matrix::Matrix(const Quat& q)
 *
 * Constructs a rotation matrix from (possibly non-unit) quaternion.
 * Assumes matrix is used to multiply column vector on the left:
 *	Vnew = matrix Vold. Works correctly for right-handed
 * coordinate system and right-handed rotations.
 *
 * @see Matrix::Set
 */
Matrix::Matrix(const Quat& q) : SharedObj()
{	
	Set(q);
}

/*!
 * @fn void Matrix::Set(const Quat& q)
 *
 * Constructs a rotation matrix from (possibly non-unit) quaternion.
 * Assumes matrix is used to multiply column vector on the left:
 *	Vnew = matrix Vold. Works correctly for right-handed
 * coordinate system and right-handed rotations.
 *
 * @see Matrix::Matrix
 */
void Matrix::Set(const Quat& q)
{
    float data[4][4];
	data[0][3] = data[1][3] = data[2][3] = 0.0;
    data[3][0] = data[3][1] = data[3][2] = 0.0;
	data[3][3] = 1.0f;

	float	s = 2.0f / q.LengthSquared();
	float	xx = q.x * q.x;
	float	xy = q.x * q.y;
	float	xz = q.x * q.z;
	float	xw = q.x * q.w;
	float	yy = q.y * q.y;
	float	yz = q.y * q.z;
	float	yw = q.y * q.w;
	float	zz = q.z * q.z;
	float	wz = q.z * q.w;
	float	ww = q.w * q.w;

	//	1-2(yy+zz)	2(xy+wz)	2(xz-yw)
	//	2(xy-wz)	1-2(xx+zz)	2(yz+xw)
	//	2(xz+yw)	2(yz-xw)	1-2(xx+yy)
	//
	data[0][0] = 1.0f - s * (yy + zz);
	data[1][0] = s * (xy + wz);
	data[2][0] = s * (xz - yw);
	data[0][1] = s * (xy - wz);
	data[1][1] = 1.0f - s * (xx + zz);	
	data[2][1] = s * (yz + xw);
	data[0][2] = s * (xz + yw);
	data[1][2] = s * (yz - xw);
	data[2][2] = 1.0f - s * (xx + yy);
	SetMatrix(&(data[0][0]));
	SetChanged(true);

#ifdef _DEBUG
	if (Pose::Debug > 2)
	{
		Quat tmp(*this);

		if (tmp.Distance(q))
			VX_WARNING(("Matrix from Quaterion error (%0.3f, %0.3f, %0.3f, %0.3f) != (%0.3f, %0.3f, %0.3f, %0.3f))\n",
						tmp.x, tmp.y, tmp.z, tmp.w, q.x, q.y, q.x, q.w));
	}
#endif

}

/*!
 * @fn Quat::Quat(const Vec3& axis, float angle)
 * @param axis	axis of rotation
 * @param angle	angle of rotation
 *
 * Given a unit axis [Ax Ay Az] and an \b angle about this axis,
 * the quaternion is
 * @code
 *	x = Ax * sin(angle / 2)
 *	y = Ay * sin(angle / 2)
 *	z = Az * sin(angle / 2)  
 *	w = cos(angle / 2)
 * @endcode
 *
 * @see Quat::Quat(const Matrix&) Quat::Set
 */
Quat::Quat(const Vec3& axis, float angle)
{
	Set(axis, angle); 
	Normalize();
}

/*!
 * @fn void Quat::Quat(const Vec3& v1, const Vec3& v2)
 * @param v1	first vector
 * @param v2	second vector
 *
 * Given two vectors, this constructor produces a quaternion
 * that describes the rotation to get from v1 to v2.
 * The function computes the angle and axis as follows:
 * @code
 *	angle = v1 . v2 / |v1| |v2|
 *	axis = v1 x v2
 * @endcode
 * The angle and axis are used to initialize the quaternion.
 *
 * @see Quat::Quat(const Vec3&, float) Quat::Set(const Vec3&, const Vec3&)
 */
Quat::Quat(const Vec3& v1, const Vec3& v2)
{
	Set(v1, v2);
}

/*!
 * @fn void Quat::Set(const Matrix& mtx)
 * @param mtx	matrix to make quaternion from
 *
 * Constructs a quaternion from the rotation component
 * of the input matrix.
 *
 * @see Quat::Quat(const Matrix&)
 */
Quat::Quat(const Matrix& trans)
{
	Set(trans);
}

Quat::Quat(const Vec3* vecs)
{
	Matrix mtx;

	mtx.RotationMatrix(vecs);
	Set(mtx);
}

/*!
 * @fn bool Vec4::IsEmpty() const
 *
 * @return \b true if the vector is initialized to zero, else \b false
 */
bool Quat::IsEmpty() const
{
	float t = x + y + z;

	if (fabs(t) > VX_SMALL_NUMBER)
		return false;
	if (fabs(w) < VX_SMALL_NUMBER)
		return true;
	return fabs(1.0f - w) < VX_SMALL_NUMBER;
}

/*!
 * @fn void Quat::Set(const Vec3& v1, const Vec3& v2)
 * @param v1	first vector
 * @param v2	second vector
 *
 * Given two vectors, this function initializes a quaternion
 * that describes the rotation to get from v1 to v2.
 * The function computes the angle and axis as follows:
 * @code
 *	angle = v1 . v2 / |v1| |v2|
 *	axis = v1 x v2
 * @endcode
 * The angle and axis are used to initialize the quaternion.
 *
 * @see Quat::Quat(const Vec3&, float)
 */
void Quat::Set(const Vec3& v1, const Vec3& v2)
{
	if (v1.IsEmpty() || v2.IsEmpty())
	{
		Set(0.0f, 0.0f, 0.0f, 1.0f);
		return;
	}
	Vec3	t1(v1); t1.Normalize();
	Vec3	t2(v2); t2.Normalize();
	float	d = t1.Dot(t2);
	float	angle = acosf(d);
	Vec3	axis = t1.Cross(t2);
	float	s = sinf(angle / 2.0f);

	if (fabs(angle) < VX_SMALL_NUMBER)		// no rotation?
	{
		Set(0.0f, 0.0f, 0.0f, 1.0f);
		return;
	}
	if (fabs(1.0f + d) < VX_SMALL_NUMBER)	// 180 degrees?
	{
		if (fabs(t1.x - t2.x) > VX_SMALL_NUMBER)
		{
			axis.z = t1.z;					// rotate v1 90 degrees
			axis.x = t1.y;					// about Z to get axis
			axis.y = t1.x;
		}
		else								// rotate v1 90 degrees
		{									// about Y to get axis
			axis.x = t1.x;
			axis.y = t1.z;
			axis.z = t1.y;
		}
	}
	axis.Normalize();
	x = axis.x * s;							// angle/axis convert to quaternion
	y = axis.y * s;
	z = axis.z * s;
	w = cosf(angle / 2.0f);
}

/*!
 * @fn void Quat::Set(const Vec3& axis, float angle)
 * @param axis	axis of rotation
 * @param angle	angle of rotation
 *
 * Given a unit axis [Ax Ay Az] and an \b angle about this axis,
 * the quaternion is
 * @code
 *	x = Ax * sin(angle / 2)
 *	y = Ay * sin(angle / 2)
 *	z = Az * sin(angle / 2)  
 *	w = cos(angle / 2)
 * @endcode
 *
 * @see Quat::Quat(const Vec3&, float)
 */
void Quat::Set(const Vec3& axis, float angle)
{
	// divisor for normalization of input axis
	float n = sqrtf(axis.x * axis.x +  axis.y * axis.y + axis.z * axis.z);

	// if axis is undefined (i.e) length very close to 0, set 
	// quat to angle of zero, (0,0,0,1)
	if (n < VX_EPSILON)
	{
		x = y = z = 0.0f;
		w = 1.0f;
		return;
	}

	float s = sinf(angle / 2.0f);	// normalize axis and scale appropriately
	x = axis.x * s / n;
	y = axis.y * s / n;
	z = axis.z * s / n;
	w = cosf(angle / 2.0f);			// scale "real" part
}

/*!
 * @fn void Quat::Set(const Matrix& mtx)
 * @param mtx	matrix to make quaternion from
 *
 * Constructs a quaternion from the rotation component
 * of the input matrix.
 * w = (1 + m00 + m11 + m22) / 2
 * x = (m21 - m12) / (4 * w)
 * y = (m02 - m20) / (4 * w)
 * z = (m10 - m01) / (4 * w)
 *
 * @see Quat::Quat(const Matrix&)
 */
void Quat::Set(const Matrix& trans)
{
	const float (*m)[4][4] = (float (*)[4][4]) trans.GetMatrix();
    float trace = (*m)[0][0] + (*m)[1][1] + (*m)[2][2];
    float s;

    if (trace > 0.0f)
    {
        x = (*m)[2][1] - (*m)[1][2];
        y = (*m)[0][2] - (*m)[2][0];
        z = (*m)[1][0] - (*m)[0][1];
		w = s = trace + 1.0f;
   }
    else if (((*m)[0][0] > (*m)[1][1]) && ((*m)[0][0] > (*m)[2][2])) 
    {
		x = s = 1.0f + (*m)[0][0] - (*m)[1][1] - (*m)[2][2];
		y = (*m)[1][0] + (*m)[0][1];
 		z = (*m)[0][2] + (*m)[2][0];
		w = (*m)[2][1] - (*m)[1][2]; 
	}
	else if ((*m)[1][1] > (*m)[2][2])
	{
		x = (*m)[0][1] + (*m)[1][0];
		y = s = 1.0f + (*m)[1][1] - (*m)[0][0] - (*m)[2][2];
		z = (*m)[1][2] + (*m)[2][1];
		w = (*m)[0][2] - (*m)[2][0];
	}
	else
	{
		x = (*m)[0][2] + (*m)[2][0];
		y = (*m)[1][2] + (*m)[2][1];
		z = s = 1.0f + (*m)[2][2] - (*m)[0][0] - (*m)[1][1];
		w = (*m)[1][0] - (*m)[0][1];
	}
	*this *= 0.5f / sqrtf(s);
}

/*!
 * @fn void Quat::Mul(const Quat& ql, const Quat& qr)
 *
 * Return quaternion product \b ql * \b qr in this quaternion.
 *
 * @note Order is important!
 * To combine rotations, use the product Mul(qSecond, qFirst),
 * which gives the effect of rotating by qFirst then qSecond.
 *
 * @see Matrix::Multiply Matrix::Rotate
 */
void Quat::Mul(const Quat& qL, const Quat& qR)
{
	// Multiply Q0 = (a0, V0) and Q1 = (a1, V1), Q = Q0*Q1
	// Q = (a = (a0*a1 - V0 dot V1),  V = (a0V1 + a1V0 + v0 cross V1 ))
	float tmpw = qL.w * qR.w - qL.x * qR.x - qL.y * qR.y - qL.z * qR.z;
	float tmpx = qL.w * qR.x + qL.x * qR.w + qL.y * qR.z - qL.z * qR.y;
	float tmpy = qL.w * qR.y + qL.y * qR.w + qL.z * qR.x - qL.x * qR.z;
	float tmpz = qL.w * qR.z + qL.z * qR.w + qL.x * qR.y - qL.y * qR.x;
	w = tmpw;
	x = tmpx;
	y = tmpy;
	z = tmpz;
}

Quat& Quat::operator*=(const Quat& q)
{
	// Multiply Q0 = (a0, V0) and Q1 = (a1, V1), Q = Q0*Q1
	// Q = (a = (a0*a1 - V0 dot V1),  V = (a0V1 + a1V0 + v0 cross V1 ))
	float tmpw = w * q.w - x * q.x - y * q.y - z * q.z;
	float tmpx = w * q.x + x * q.w + y * q.z - z * q.y;
	float tmpy = w * q.y + y * q.w + z * q.x - x * q.z;
	float tmpz = w * q.z + z * q.w + x * q.y - y * q.x;
	w = tmpw;
	x = tmpx;
	y = tmpy;
	z = tmpz;
	return *this;
}

/*!
 * @fn Quat::Slerp(const Quat& q1, const Quat& q2, float alpha)
 * @param q1	First quaternion for interpolation
 * @param q2	Second quaternion for interpolation
 * @param alpha	interpolation value between 0 and 1
 *
 * Spherical linear interpolation between two quaternions.
 * We will interpolate between q1 and -q2 instead of q1 and q2
 * if the distance is smaller so that the interpolation will
 * be the right way around the arc.
 *
 */
void Quat::Slerp(const Quat& q1, const Quat& q2, float t)
{
	Quat	P(q1);
	Quat	Q;
	float	d = P.DistanceSquared(q2);

	if (d > 2)
		Q = q2 * -1;
	else 
		Q = q2;

    float fCos = P.Dot(Q);
    float omega = acosf(fCos);

    if ((fCos > 1.0f) || (fabs(omega) < VX_EPSILON))
	{
		*this = P;
        return;
	}

    float sinom = sinf(omega);
    float fInvSin = 1.0f / sinom;
    float sclp = sinf((1.0f - t) * omega) * fInvSin;
    float sclq = sinf(t * omega) * fInvSin;
	Quat tmp(Q);

	*this = P;
	*this *= sclp;
	tmp *= sclq;
	*this += tmp;
}

/*!
 * @fn void Quat::Conjugate()
 *
 * Conjugates a quaternion:
 *		[ x y z w ] -> [ -x -y -z  w ]
 */
void Quat::Conjugate()
{
	x = -x;
	y = -y;
	z = -z;
	w = w;
}

/*!
 * @fn void Quat::Invert()
 *
 * Inverts a quaternion:
 *		[ x y z w ] -> [ -x -y -z  w ] / (x*x + y*y + z*z + w*w)
 */
void Quat::Invert()
{
	float v = x*x + y*y + z*z + w*w;

	x = -x;
	y = -y;
	z = -z;
	w = w;
	if (v > FLT_EPSILON)
	{
		v = sqrtf(v);
		x /= v;
		y /= v;
		z /= v;
		w /= v;
	}
}

void Quat::Normalize()
{
	double n = x * x + y * y + z * z;
	double s = n + w * w;

	if (s == 1.0)
		return;
	// Since the rotations are represented by unit quaternions,
	// this case represents a quaternion (for our purposes) that
	// is WAY out of whack... Set the quaternion to a rotation of zero.
	else if ((n < FLT_EPSILON) || (s < VX_EPSILON))
	{ 
		w = 1.0f;
		x = y = z = 0.0f;
		return;
	}
	s = sqrt(s);
	x = (float) (x / s);					// normalize and scale <x,y,z>
	y = (float) (y / s);
	z = (float) (z / s);
	w = (float) (w / s);
}

void Quat::Squad(const Quat & p, const Quat & a, const Quat & b, const Quat & q, float t)
{
	float k = (float)2.0 * ((float)1.0 -t) * t;
	Quat r1, r2;
	r1.Slerp(p, q, t);
	r2.Slerp(a, b, t);
	Slerp(r1, r2, k);
}

/*!
 * @fn void Quat::GetEulerAngles(Vec3& vec) const
 * @param vec	vector to get Euler rotation angles
 *
 * Converts this quaternion to Euler angles and stores the X, Y, Z
 * rotation angles in the components of the input vector.
 * The conversion formula is:
 * @code
 *	attitude:	vec.x = arcsin( 2*(x*y + z*w) )
 *	heading:	vec.y = arctan( 2*(w*y - x*z) / 1-2*(z*z + y*y) )
 *	bank:		vec.z = arctan( 2*(w*x - y*z) / 1-2*(x*x + y*y) )
 * @endcode
 */
void Quat::GetEulerAngles(Vec3& euler_angles) const
{
	euler_angles.x = 2.0f * (x * y + z * w);
	euler_angles.x = asin(euler_angles.x);
	euler_angles.y = 2.0f * (y * w - x * z) / (1.0f - 2.0f * (y * y + z * z));
	euler_angles.y = atan(euler_angles.y);
	euler_angles.z = 2.0f * (w * x - y * z) / (1.0f - 2.0f * (x * x + y * y)); 
	euler_angles.z = atan(euler_angles.z);
	 
	if ((x * y  + z * w)  == 0.5f)  
	{  
		euler_angles.y = (float) (2.0f * atan2(y, w));  
		euler_angles.z = 0;      
	}  
	else if ((x * y  + z * w) == -0.5f)  
	{  
		euler_angles.y = (float) (-2.0f * atan2(y, w));  
		euler_angles.z = 0;      
	}
}

}	// end Vixen