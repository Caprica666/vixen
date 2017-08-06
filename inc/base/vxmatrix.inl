/****
 *
 * Inlines for Matrix
 *
 ****/

namespace Vixen {

FORCE_INLINE void
Matrix::Set(int i1, int i2, float v)
{
	VX_ASSERT( i1 >=0 && i1 <= 4 && i2 >=0 && i2 <= 4 );
	data[i1][i2] = v;
	identity = false;
}

FORCE_INLINE float
Matrix::Get(int i1, int i2) const
{
	VX_ASSERT( i1 >=0 && i1 <= 4 && i2 >=0 && i2 <= 4 );
	return data[i1][i2];
}

FORCE_INLINE const float*	Matrix::GetMatrix() const
{
	return (const float*) data;
}

FORCE_INLINE const Matrix*	Matrix::GetIdentity()
{
	if (s_IdentityMatrix == NULL)
	{
		s_IdentityMatrix = new Matrix;
		s_IdentityMatrix->IncUse();
	}
	return s_IdentityMatrix;
}

FORCE_INLINE bool Matrix::operator==(const Matrix &src) const
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
		{
			float t1 = Get(i, j);
			float t2 = src.Get(i, j);
			float tmp = t1 - t2;

			if (fabs(tmp) > VX_EPSILON)
				return false;
		}
	return true;
}

FORCE_INLINE bool Matrix::operator!=(const Matrix &src) const
{
	return !operator==(src);
}

FORCE_INLINE void	Matrix::GetTranslation(Vec3& trans) const
{
	trans.x = Get(0, 3);
	trans.y = Get(1, 3);
	trans.z = Get(2, 3);
}

FORCE_INLINE void	Matrix::SetTranslation(const Vec3& trans)
{
	Set(0, 3, trans.x);
	Set(1, 3, trans.y);
	Set(2, 3, trans.z);
	identity = false;
}

FORCE_INLINE void	Matrix::GetScale(Vec3& scale) const
{
	Vec3 tmp_vec;
 
	tmp_vec.Set(Get(0,0), Get(1,0), Get(2,0)); 
	scale.x = tmp_vec.Length();
	tmp_vec.Set(Get(0,1), Get(1,1), Get(2,1)); 
	scale.y = tmp_vec.Length();
	tmp_vec.Set(Get(0,2), Get(1,2), Get(2,2)); 
	scale.z = tmp_vec.Length();
}

/*!
 * @fn void Vec3::TransformVector(const Matrix& mtx, const Vec3& src)
 * @param mtx	4x4 matrix to multiply source vector by.
 * @param src	source vector.
 *
 *	Transforms the source vector by the given matrix.
 *	The vector is treated as a 3x1 column vector and is
 *	pre-multiplied by the 3x3 part of the 4x4 matrix.
 *	Translation and projection components of the matrix are ignored.
 *
 * @code
 *		| M00 M10 M20 | |x|
 *		| M01 M11 M21 | |y|
 *		| M02 M12 M22 | |z|
 * @endcode
 *
 * @see Matrix::Transform Vec3::operator*= Vec3::Transform
 */
FORCE_INLINE void Matrix::Transform(const Vec3 &tmp, Vec3 &dst) const
{
	float x = Dot3(tmp, data[0]) + data[0][3];
	float y = Dot3(tmp, data[1]) + data[1][3];
	float z = Dot3(tmp, data[2]) + data[2][3];
	dst.Set(x, y, z);
}

FORCE_INLINE void Matrix::TransformVector(const Vec3 &src, Vec3 &dst) const
{
	float x = Dot3(src, data[0]);
	float y = Dot3(src, data[1]);
	float z = Dot3(src, data[2]);
	dst.Set(x, y, z);
}

FORCE_INLINE void	Vec3::TransformVector(const Matrix& mtx, const Vec3& src)
{
	mtx.TransformVector(src, *this);
}

} // end Vixen