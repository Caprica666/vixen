
/****
 *
 * Matrix: 3D transformation matrix class
 *
 ****/
#include "vixen.h"

namespace Vixen
{

Messenger& operator<<(Messenger& d, const Col4& c)
{	return d << c.r << c.g << c.b << c.a; }

Messenger& operator>>(Messenger& d, Col4& c)
{	return d >> c.r >> c.g >> c.b >> c.a; }

Messenger& operator<<(Messenger& d, const Vec2& v)
{	return d << v.x << v.y; }

Messenger& operator>>(Messenger& d, Vec2& v)
{	return d >> v.x >> v.y; }

Messenger& operator<<(Messenger& d, const Vec3& v)
{	return d << v.x << v.y << v.z; }

Messenger& operator>>(Messenger& d, Vec3& v)
{	return d >> v.x >> v.y >> v.z; }

Messenger& operator<<(Messenger& d, const Vec4& v)
{	return d << v.x << v.y << v.z << v.w; }

Messenger& operator>>(Messenger& d, Vec4& v)
{	return d >> v.x >> v.y >> v.z >> v.w; }


VX_IMPLEMENT_CLASSID(Matrix, SharedObj, VX_Matrix);

Matrix* Matrix::s_IdentityMatrix = NULL;

static const TCHAR* opnames[] =
{	TEXT("Identity"), TEXT("SetMatrix"), };

const TCHAR** Matrix::DoNames = opnames;

static void Up_triangle_Inverse_Xform(const float [4][4], float [4][4]);
static void Down_triangle_Inverse_Xform(const float [4][4], float [4][4]);
static void Full_Inverse_Xform3(const float [4][4], float [4][4]);

Matrix::Matrix() : SharedObj()
{
	identity = false;
	Identity();
}

Matrix::Matrix(const float *foreign_data) : SharedObj()
{
	assert( 0 != foreign_data );

	identity = false;
	memcpy(data, foreign_data, sizeof(data));
	SetChanged(true);
}

Matrix::Matrix(const Matrix& rhs) : SharedObj(rhs)
{
	memcpy(data, rhs.data, sizeof(data));
	identity = rhs.identity;
}

/*!
 * @fn Vec4 Matrix::GetRotationAxis() const
 *
 * Returns the axis and angle of rotation about
 * that axis as a vector. The x, y, z components
 * of the vector specify the axis and w gives
 * the rotation angle in radians.
 *
 *@see Quat::Quat Matrix::Rotate Matrix::RotationMatrix
 */
Vec4 Matrix::GetRotationAxis() const
{
	Vec3 V;
	double theta, factor;
		// convert from cosine matrix to axis-angle
	theta = acos((Get(0, 0) + Get(1, 1) + Get(2, 2) - 1.0f) * 0.5f);
	factor = sin(theta);
	if ( fabs( theta) < 0.0003)
	{	// theta < 1 minute of arc
		V.x = 0.0f;	V.y = 0.0f;	V.z = 0.0f;
	}
	else
	{
		factor = 1.0 / (2.0 * factor);
		V.x = (float) (factor * (Get(2, 1) - Get(1, 2)));
		V.y = (float) (factor * (Get(0, 2) - Get(2, 0)));
		V.z = (float) (factor * (Get(1, 0) - Get(0, 1)));
	}
	return Vec4(V.x, V.y, V.z, float(theta));
}

void	Matrix::GetRotation(Vec3* vecs) const
{
	vecs[0].Set(Get(0, 0), Get(1, 0), Get(2, 0));
	vecs[1].Set(Get(0, 1), Get(1, 1), Get(2, 1));
	vecs[2].Set(Get(0, 2), Get(1, 2), Get(2, 2));
}

/*!
 * @fn void Matrix::Touch()
 *
 * Notifies the transform that you have changed it's data.
 * In the base class, it sets the SharedObj::CHANGED flag.
 * This flag may be checked by clients of this Matrix
 * to determine it has been updated. Model objects
 * use this capability to determine their internal
 * transforms have been changed during scene graph display.
 *
 */
void Matrix::Touch()
	{ SetChanged(true); }

/****
 *
 * class Matrix override for SharedObj::Print
 * Prints a Matrix in ASCII format on standard output
 *
 ****/
DebugOut& Matrix::Print(DebugOut& dbg, int opts) const
{
	if ((opts == 0) || (opts & (PRINT_Data | PRINT_Attributes)))
	{
		SharedObj::Print(dbg, opts & ~PRINT_Trailer);
		for (int i = 0; i < 4; ++i)
		{
			dbg << "\t";
			for (int j = 0; j < 4; ++j)
				dbg << Get(i, j) << "\t";
			endl(dbg);
		}
		SharedObj::Print(dbg, opts & PRINT_Trailer);
	}
	else SharedObj::Print(dbg, opts);
	return dbg;
}

/*!
 * @fn void Matrix::Identity()
 *
 * Replaces this matrix with the identity matrix.
 * The previous contents of the matrix are lost.
 *
 * @see Matrix::SetMatrix Matrix::Matrix Matrix::Identity
 */
void Matrix::Identity()
{
	if (identity) 
		return;
	else
		identity = true;
	SetChanged(true);
#if 1
	data[0][1] = 0.0f;
	data[0][2] = 0.0f;
	data[0][3] = 0.0f;    
	data[3][3] = 1.0f;
	data[1][0] = 0.0f;
	data[1][2] = 0.0f;
	data[1][3] = 0.0f;
	data[3][3] = 1.0f;
	data[2][0] = 0.0f;
	data[2][1] = 0.0f;
	data[2][2] = 1.0f;
	data[3][3] = 1.0f;
	data[2][3] = 0.0f;
	data[1][1] = 1.0f;
	data[3][0] = 0.0f;
	data[3][1] = 0.0f;
	data[3][2] = 0.0f;
	data[0][0] = 1.0f;
	data[3][3] = 1.0f;
#else
	data[0][0] = 1.0f;
	data[0][1] = 0.0f;
	data[0][2] = 0.0f;
	data[0][3] = 0.0f;
	data[1][0] = 0.0f;
	data[1][1] = 1.0f;
	data[1][2] = 0.0f;
	data[1][3] = 0.0f;
	data[2][0] = 0.0f;
	data[2][1] = 0.0f;
	data[2][2] = 1.0f;
	data[2][3] = 0.0f;
	data[3][0] = 0.0f;
	data[3][1] = 0.0f;
	data[3][2] = 0.0f;
	data[3][3] = 1.0f;
#endif
}

/*!
 * @fn void Matrix::Zero()
 *
 * Sets all the matrix elements to zero.
 * This is different than the identity matrix
 * which puts 1s along the diagonal.
 *
 * @see Matrix::Identity Matrix::SetMatrix
 */
void Matrix::Zero()
{
	memset(data, 0, sizeof(data) );
	identity = false;
	SetChanged(true);
}

/*!
 * @fn void Matrix::SetMatrix(const float* src)
 * @param data 	Array of 16 floating values that defines a 4x4 matrix.
 *			If NULL, the matrix is initialized as identity.
 *
 * Updates the 4x4 matrix data area of the matrix.
 *
 * @see Matrix::Identity Matrix::Matrix
 */
void Matrix::SetMatrix(const float* src)
{
	VX_STREAM_BEGIN(s)
	if (src == NULL)
		*s << OP(VX_Matrix, MATRIX_Identity) << this;
	else
	{
		*s << OP(VX_Matrix, MATRIX_SetMatrix) << this;
		s->Output((float*) src, 16);
	}
	VX_STREAM_END(  )

	SetChanged(true);
	identity = false;
	memcpy(data, src, sizeof(data));
}

void Matrix::XAngleMatrix(float angle)
{
	float	cost = cosf(angle);
	float	sint = sinf(angle);

	data[0][0] = 1.0f;
	data[0][1] = 0.0f;
	data[0][2] = 0.0f;
	data[1][0] = 0.0f;
	data[1][1] = cost;
	data[1][2] = -sint;
	data[2][0] = 0.0f;
	data[2][1] = sint;
	data[2][2] = cost;
	identity = false;
	SetChanged(true);
}

void Matrix::YAngleMatrix(float angle)
{
	float	cost = cosf(angle);
	float	sint = sinf(angle);

	data[0][0] = cost;
	data[0][1] = 0.0f;
	data[0][2] = sint;
	data[1][0] = 0.0f;
	data[1][1] = 1.0f;
	data[1][2] = 0.0f;
	data[2][0] = -sint;
	data[2][1] = 0.0f;
	data[2][2] = cost;
	identity = false;
	SetChanged(true);
}

void Matrix::ZAngleMatrix(float angle)
{
	float	cost = cosf(angle);
	float	sint = sinf(angle);

	data[0][0] = cost;
	data[0][1] = -sint;
	data[0][2] = 0.0f;
	data[1][0] = sint;
	data[1][1] = cost;
	data[1][2] = 0.0f;
	data[2][0] = 0.0f;
	data[2][1] = 0.0f;
	data[2][2] = 1.0f;
	identity = false;
	SetChanged(true);
}

/*!
 * @fn void Matrix::RotationMatrix(const Vec3& axis, float angle)
 * @param axis	A unit vector in the direction of the axis to rotate about.
 *				The rotation axis is assumed to go through the origin.
 * @param angle	Amount to rotate by (in radians)
 *
 * Replaces this matrix with a rotation matrix that rotates
 * \b angle radians about the specified axis
 *
 * @see Matrix::Rotate Model::Rotate Model::Turn
 */
void Matrix::RotationMatrix(const Vec3& rotaxis, float angle)
{
    float cost = cosf(angle);
    float sint = sinf(angle);
	identity = false;
	SetChanged(true);
    data[0][3] = 0.0f;
    data[1][3] = 0.0f;
    data[2][3] = 0.0f;
    data[3][3] = 1.0f;
    data[3][0] = 0.0f;
    data[3][1] = 0.0f;
    data[3][2] = 0.0f;
	if (rotaxis == Vec3(1, 0, 0))
		XAngleMatrix(angle);
    else if (rotaxis == Vec3(0, 1, 0))
		YAngleMatrix(angle);
    else if (rotaxis == Vec3(0, 0, 1))
		ZAngleMatrix(angle);
    else
	{
		Vec3 axis(rotaxis);
		axis.Normalize();

        float mcos = 1.0f - cost;
        float x2 = axis.x * axis.x;
        float y2 = axis.y * axis.y;
        float z2 = axis.z * axis.z;
        float xym = axis.x * axis.y * mcos;
        float xzm = axis.x * axis.z * mcos;
        float yzm = axis.y * axis.z * mcos;
        float asin = axis.x * sint;
        float bsin = axis.y * sint;
        float csin = axis.z * sint;
        data[0][0] = x2 * mcos + cost;
        data[0][1] = xym - csin;
        data[0][2] = xzm + bsin;
        data[1][0] = xym + csin;
        data[1][1] = y2 * mcos + cost;
        data[1][2] = yzm - asin;
        data[2][0] = xzm - bsin;
        data[2][1] = yzm + asin;
        data[2][2] = z2 * mcos + cost;
    }
}


/*!
 * @fn void Matrix::RotationMatrix(const Quat& q)
 * @param q	Quaternion specifying rotation
 *
 * Replaces this matrix with a rotation matrix constructed
 * from the input quaternion.
 *
 * @see Quat Matrix::Rotate Model::Rotate Model::Turn
 */
void Matrix::RotationMatrix(const Quat& q)
{
	Set(q);
}

/*!
 * @fn void Matrix::RotationMatrix(const VXVecs* vecs)
 * @param vecs	array of 3 linearly independent vectors
 *
 * Replaces this matrix with a rotation matrix constructed
 * from the input vectors
 *
 * @see Quat Matrix::GetRotation Matrix::Rotate Model::Rotate Model::Turn
 */
void Matrix::RotationMatrix(const Vec3* vecs)
{
    for (int c = 0; c < 3; c++)
    {
        Set(0, c, vecs[c].x);
        Set(1, c, vecs[c].y);
        Set(2, c, vecs[c].z);
    }
}

/*!
 * @fn void Matrix::Rotate(const Vec3& axis, float angle)
 * @param axis	A unit vector giving the direction
 *				of the axis to rotate about. The rotation axis is
 *				assumed to go through the origin.
 * @param angle	Amount to rotate by (in radians)
 * @param quat	Quaternion specifying rotation axis and angle
 *
 * Pre-multiplies this matrix with a rotation matrix about given axis.
 *
 * @see Matrix::Translate Matrix::Scale Model::Rotate Model::Turn
 */
void Matrix::Rotate(const Vec3& axis, float angle)
{
	if (angle == 0.0)
		return;
	Matrix tmp;
	tmp.RotationMatrix(axis, angle);
	PreMul(tmp);
}

/*!
 * @fn void Matrix::Rotate(const Quat& q)
 * @param quat	Quaternion specifying rotation axis and angle
 *
 * Pre-multiplies this matrix with a rotation matrix constructed
 * from the input quaternion.
 *
 * @see Quat Matrix::Translate Matrix::Scale Model::Rotate Model::Turn
 */
void Matrix::Rotate(const Quat& q)
{	
	Matrix tmp;
	tmp.RotationMatrix(q);
	PreMul(tmp);
}

/*!
 * @fn void Matrix::TranslationMatrix(const Vec3 &v)
 * @param v	translation vector
 *
 * Replaces this matrix with a translation matrix
 * @code
 *	| 1 0 0 v.x |
 *	| 0 1 0 v.y |
 *	| 0 0 1 v.z |
 *	| 0 0 0  1  |
 * @endcode
 *
 *
 * @see Matrix::Translate Model::Translate Model::Move
 */
void Matrix::TranslationMatrix(const Vec3 &v)
{
		//jb - try to save some time
	if (identity)
	{
		data[0][3] = v.x;
		data[1][3] = v.y;
		data[2][3] = v.z;
	}
	else
	{
		data[0][0] = 1.0f;
		data[0][1] = 0.0f;
		data[0][2] = 0.0f;
		data[0][3] = v.x;
		data[1][0] = 0.0f;
		data[1][1] = 1.0f;
		data[1][2] = 0.0f;
		data[1][3] = v.y;
		data[2][0] = 0.0f;
		data[2][1] = 0.0f;
		data[2][2] = 1.0f;
		data[2][3] = v.z;
		data[3][0] = 0.0f;
		data[3][1] = 0.0f;
		data[3][2] = 0.0f;
		data[3][3] = 1.0f;
	}
	identity = false;
	SetChanged(true);
}

/*!
 * @fn void Matrix::Translate(float x, float y, float z)
 * @param x	amount to translate in X direction
 * @param y	amount to translate in Y direction
 * @param z	amount to translate in Z direction
 *
 * Pre-multiplies this matrix with a translation matrix
 * @code
 *	| 1 0 0 x |
 *	| 0 1 0 y |
 *	| 0 0 1 z |
 *	| 0 0 0 1 |
 * @endcode
 *
 * @see Matrix::Translate Model::Translate Model::Move
 */
void Matrix::Translate(float x, float y, float z)
{
	//jb - took out one layer of function calls
	Matrix tmp;
	tmp.TranslationMatrix(Vec3(x, y, z));
	PreMul(tmp);
}

/*!
 * @fn void Matrix::Translate(const Vec3 &v)
 * @param v	translation vector
 *
 * Pre-multiplies this matrix with a translation matrix
 * @code
 *	| 1 0 0 v.x |
 *	| 0 1 0 v.y |
 *	| 0 0 1 v.z |
 *	| 0 0 0  1  |
 * @endcode
 *
 * @see Matrix::Translate Model::Translate Model::Move
 */
void Matrix::Translate(const Vec3& p)
{
	Matrix tmp;
	tmp.TranslationMatrix(p);
	PreMul(tmp);
}

/*!
 * @fn void Matrix::ScaleMatrix(const Vec3& v)
 * @param v scaling vector
 *
 * Replaces this matrix by a scaling matrix.
 * @code
 *	| v.x 0   0   0 |
 *	| 0   v.y 0   0 |
 *	| 0   0   v.z 0 |
 *	| 0   0   0  1 |
 * @endcode
 *
 * @see Matrix::Scale Model::Scale Model::Size
 */
void Matrix::ScaleMatrix(const Vec3 &s)
{
	if (identity)
	{
		data[0][0] = s.x;
		data[1][1] = s.y;
		data[2][2] = s.z;
	}
	else
	{
		data[0][0] = s.x;
		data[0][1] = 0.0f;
		data[0][2] = 0.0f;
		data[0][3] = 0.0f;
		data[1][0] = 0.0f;
		data[1][1] = s.y;
		data[1][2] = 0.0f;
		data[1][3] = 0.0f;
		data[2][0] = 0.0f;
		data[2][1] = 0.0f;
		data[2][2] = s.z;
		data[2][3] = 0.0f;
		data[3][0] = 0.0f;
		data[3][1] = 0.0f;
		data[3][2] = 0.0f;
		data[3][3] = 1.0f;
	}
	identity = false;
	SetChanged(true);
}

/*!
 * @fn void Matrix::Scale(float x, float y, float z)
 * @param x	amount to scale in X direction
 * @param y	amount to scale in Y direction
 * @param z	amount to scale in Z direction
 *
 * Pre multiplies this matrix by a scaling matrix.
 * @code
 *	this =	| x 0 0 0 | * this
 *			| 0 y 0 0 |
 *			| 0 0 z 0 |
 *			| 0 0 0 1 |
 * @endcode
 *
 * @see Matrix::Rotate Matrix::Translate Model::Scale Model::Size
 */
void Matrix::Scale(float x, float y, float z)
{
	//jb - removing function calls
	//Scale(Vec3(x, y, z));
	Matrix tmp;
	tmp.ScaleMatrix(Vec3(x, y, z));
	PreMul(tmp);
}

/*!
 * @fn void Matrix::Scale(const Vec3& v)
 * @param v scaling vector
 *
 * Pre-multiplies this matrix by a scaling matrix.
 * @code
 *	this =	| v.x 0	 0   0 | * this
 *			| 0	 v.y 0   0 |
 *			| 0   0  v.z 0 |
 *			| 0   0  0   1 |
 * @endcode
 *
 * @see Matrix::Rotate Matrix::Translate Model::Scale Model::Size
 */
void Matrix::Scale(const Vec3& p)
{
	Matrix tmp;
	tmp.ScaleMatrix(p);
	PreMul(tmp);
}

/*!
 * @fn void Matrix::LookAt(const Vec3& v, float twist)
 * @param lookat	Point to look at. The vector from the origin to
 *					this point defines the line of sight.
 * @param twist		Amount to twist around the Z axis.
 *
 * Replaces this matrix with one which orients to
 * to look at a specific point.
 *
 * @see Model::LookAt Matrix::Rotate
 */
void Matrix::LookAt(const Vec3& v, float twist)
{
    float l2, l3, sintheta, sinphi, costheta, cosphi;
    Matrix tmp;

    l2 = sqrtf((float) ((v.x * v.x) + (v.z * v.z)));
    l3 = sqrtf((float) ((v.x * v.x) + (v.y * v.y) + (v.z * v.z)));
	Identity();
    if (l2 != 0.0)
	{
        sintheta = (-v.x) / l2;
        costheta = (-v.z) / l2;
/*
 * Rotate about Y by theta
 */
		tmp.Identity();
        tmp.Set(0, 0, costheta);
		tmp.Set(2, 2, costheta);
        tmp.Set(2, 0, -sintheta);
        tmp.Set(0, 2, sintheta);
        PostMul(tmp);
	}
    if (l3 != 0.0)
	{
        sinphi = v.y / l3;
        cosphi = l2 / l3;
/*
 * Rotate about X by phi
 */
		tmp.Identity();
        tmp.Set(1, 1, cosphi);
		tmp.Set(2, 2, cosphi);
        tmp.Set(2, 1, sinphi);
        tmp.Set(1, 2, -sinphi);
        PostMul(tmp);
	}
    tmp.RotationMatrix(Vec3(0, 0, 1), twist);
	PostMul(tmp);
}

/*!
 * @fn Matrix& Matrix::operator+=(const Matrix& m)
 * @param src	Matrix to add to this matrix
 *
 * Adds the source matrix to this matrix by adding
 * corresponding elements.
 *
 * @see Matrix::operator-= Matrix::Multiply
 */
Matrix& Matrix::operator+=(const Matrix& m)
{
	const float* src = m.GetMatrix();
	float* dst = (float*) data;

	for (int i=0; i < 16; i++)
		dst[i] += src[i];
	identity = false;
	SetChanged(true);
	return *this;
}

/*!
 * @fn Matrix& Matrix::operator-=(const Matrix& m)
 * @param src	Matrix to subtract from this matrix
 *
 * Subtracts the source matrix to this matrix by subtracting
 * corresponding elements.
 *
 * @see Matrix::operator+= Matrix::Multiply
 */
Matrix&	Matrix::operator-=(const Matrix& m)
{
	const float* src = m.GetMatrix();
	float* dst = (float*) data;

	for (int i=0; i < 16; i++)
		dst[i] -= src[i];
	identity = false;
	SetChanged(true);
	return *this;
}

/*!
 * @fn void Matrix::PreMul(const Matrix& src)
 * @param src	input multiplicand
 *
 * Multiplies the input matrix by this matrix and stores the result
 * in this matrix.  Performs a full 4x4 matrix multiplication.
 * <B>this = src * this</B>
 *
 * @see Matrix::Multiply Matrix::PostMul Matrix::operator*= Matrix::operator+= Matrix::operator-=
 */
void Matrix::PreMul(const Matrix& src)
{
	Matrix tmp(*this);
	Multiply(src, tmp);
}

/*!
 * @fn void Matrix::PostMul(const Matrix& src)
 * @param src	input multiplicand
 *
 * Multiplies this matrix by the input matrix and stores the result
 * in this matrix.  Performs a full 4x4 matrix multiplication.
 * <B>this = this * src</B>
 *
 * @see Matrix::operator*= Matrix::Multiply Matrix::PreMul Matrix::operator+= Matrix::operator-=
 */
void Matrix::PostMul(const Matrix& src)
{
	Matrix tmp(*this);
	Multiply(tmp, src);
}

/*!
 * @fn Matrix&	Matrix::operator*=(const Matrix& src)
 * @param src	input multiplicand
 *
 * Multiplies this matrix by the input matrix and stores the result
 * in this matrix.  Performs a full 4x4 matrix multiplication.
 * <B>this = this * src</B>
 *
 * @see Matrix::Multiply Matrix::PreMul Matrix::operator+= Matrix::operator-=
 */
Matrix&	Matrix::operator*=(const Matrix& src)
	{ PostMul(src); return *this; }

/*!
 * @fn void Matrix::Multiply(const Matrix &mtxA, const Matrix &mtxB)
 * @param mtxA	first multiplicand
 * @param mtxB	second multiplicand
 *
 * Multiplies the input matrices and stores the result in this matrix.
 * Performs a full 4x4 matrix multiplication.
 * <B>this = mtxA * mtxB</B>
 *
 * We interpret matrices to be in the same row/column order as C arrays.
 * To get element[i][j] of the destination, we multiply the ith row of A
 * by the jth column of B.
 * This routine does not work if two or more of the matrices are 
 * the same.  A call like Matrix::Mul(a, a, b) will compute the wrong result.
 *
 * @see Matrix::PreMul Matrix::PostMul Matrix::operator*=
 */
void Matrix::Multiply(const Matrix &inA, const Matrix &inB)
{
	const Matrix& a = (const Matrix&) inA;
	const Matrix& b = (const Matrix&) inB;

	VX_ASSERT(a.IsClass(VX_Matrix));
	VX_ASSERT(b.IsClass(VX_Matrix));

	if (a.identity && b.identity)
	{
		Identity();
		return;
	}

	identity = false;
	if (a.identity)
	{
		Copy(b);
		return;
	}
	if (b.identity)
	{
		Copy(a);
		return;
	}

	SetChanged(true);
    data[0][0]=a.data[0][0]*b.data[0][0]+a.data[0][1]*b.data[1][0]+a.data[0][2]*b.data[2][0]+a.data[0][3]*b.data[3][0];
    data[1][0]=a.data[1][0]*b.data[0][0]+a.data[1][1]*b.data[1][0]+a.data[1][2]*b.data[2][0]+a.data[1][3]*b.data[3][0];
    data[2][0]=a.data[2][0]*b.data[0][0]+a.data[2][1]*b.data[1][0]+a.data[2][2]*b.data[2][0]+a.data[2][3]*b.data[3][0];
    data[3][0]=a.data[3][0]*b.data[0][0]+a.data[3][1]*b.data[1][0]+a.data[3][2]*b.data[2][0]+a.data[3][3]*b.data[3][0];
    data[0][1]=a.data[0][0]*b.data[0][1]+a.data[0][1]*b.data[1][1]+a.data[0][2]*b.data[2][1]+a.data[0][3]*b.data[3][1];
    data[1][1]=a.data[1][0]*b.data[0][1]+a.data[1][1]*b.data[1][1]+a.data[1][2]*b.data[2][1]+a.data[1][3]*b.data[3][1];
    data[2][1]=a.data[2][0]*b.data[0][1]+a.data[2][1]*b.data[1][1]+a.data[2][2]*b.data[2][1]+a.data[2][3]*b.data[3][1];
    data[3][1]=a.data[3][0]*b.data[0][1]+a.data[3][1]*b.data[1][1]+a.data[3][2]*b.data[2][1]+a.data[3][3]*b.data[3][1];
    data[0][2]=a.data[0][0]*b.data[0][2]+a.data[0][1]*b.data[1][2]+a.data[0][2]*b.data[2][2]+a.data[0][3]*b.data[3][2];
    data[1][2]=a.data[1][0]*b.data[0][2]+a.data[1][1]*b.data[1][2]+a.data[1][2]*b.data[2][2]+a.data[1][3]*b.data[3][2];
    data[2][2]=a.data[2][0]*b.data[0][2]+a.data[2][1]*b.data[1][2]+a.data[2][2]*b.data[2][2]+a.data[2][3]*b.data[3][2];
    data[3][2]=a.data[3][0]*b.data[0][2]+a.data[3][1]*b.data[1][2]+a.data[3][2]*b.data[2][2]+a.data[3][3]*b.data[3][2];
    data[0][3]=a.data[0][0]*b.data[0][3]+a.data[0][1]*b.data[1][3]+a.data[0][2]*b.data[2][3]+a.data[0][3]*b.data[3][3];
    data[1][3]=a.data[1][0]*b.data[0][3]+a.data[1][1]*b.data[1][3]+a.data[1][2]*b.data[2][3]+a.data[1][3]*b.data[3][3];
    data[2][3]=a.data[2][0]*b.data[0][3]+a.data[2][1]*b.data[1][3]+a.data[2][2]*b.data[2][3]+a.data[2][3]*b.data[3][3];
    data[3][3]=a.data[3][0]*b.data[0][3]+a.data[3][1]*b.data[1][3]+a.data[3][2]*b.data[2][3]+a.data[3][3]*b.data[3][3];
}



void Matrix::Transform(const Vec4 &tmp, Vec4 &dst) const
{
	dst.x = Dot4(tmp, data[0]);
    dst.y = Dot4(tmp, data[1]);
    dst.z = Dot4(tmp, data[2]);
	dst.w = Dot4(tmp, data[3]); 
}

void Matrix::Transform(const Vec3 &tmp, Vec4 &dst) const
{
	dst.x = Dot3(tmp, data[0]) + data[0][3];
    dst.y = Dot3(tmp, data[1]) + data[1][3];
    dst.z = Dot3(tmp, data[2]) + data[2][3];
	dst.w = Dot3(tmp, data[3]) + data[3][3];
}

void Matrix::Transform(const Sphere& src, Sphere& dst) const
{
	Transform(src.Center, dst.Center);
	double rx, tmp;
	if (identity)
		dst.Radius = src.Radius;
	else
	{
		rx = fabs(data[0][0]);
		tmp = fabs(data[0][1]);
		if (tmp > rx) rx = tmp;
			tmp = fabs(data[0][2]);
		if (tmp > rx) rx = tmp;
			tmp = fabs(data[1][0]);
		if (tmp > rx) rx = tmp;
			tmp = fabs(data[1][1]);
		if (tmp > rx) rx = tmp;
			tmp = fabs(data[1][2]);
		if (tmp > rx) rx = tmp;
			tmp = fabs(data[2][0]);
		if (tmp > rx) rx = tmp;
			tmp = fabs(data[2][1]);
		if (tmp > rx) rx = tmp;
			tmp = fabs(data[2][2]);
		if (tmp > rx) rx = tmp;
			dst.Radius = src.Radius * (float) rx;
	}

}

/*!
 * @fn void Matrix::Transpose()
 *
 * Transposes the matrix by exchanging M[i][j] and M[j][i]
 * for i,j = 1 to 4 (all 16 elements about the diagonal).
 *
 * @see Matrix::Invert
 */
void Matrix::Transpose()
{
    float val;
    val = data[0][1];
    data[0][1] = data[1][0];
    data[1][0] = val;
    val = data[0][2];
    data[0][2] = data[2][0];
    data[2][0] = val;
    val = data[0][3];
    data[0][3] = data[3][0];
    data[3][0] = val;
    val = data[1][2];
    data[1][2] = data[2][1];
    data[2][1] = val;
    val = data[1][3];
    data[1][3] = data[3][1];
    data[3][1] = val;
    val = data[2][3];
    data[2][3] = data[3][2];
    data[3][2] = val;
	SetChanged(true);
}

/*!
 * @fn void Matrix::Invert(const Matrix& src)
 * @param src	Source matrix to invert. It's inverse
 *				replaces the destination matrix.
 *
 * Replaces this matrix with the inverse of the source matrix.
 * Currently this function does not work correctly if the source
 * and destination matrices are the same (will not compute inverse
 * in place). It also does not tell you if the matrix is singular
 * (does not have a valid inverse).
 *
 * @see Matrix::Transpose
 */
void Matrix::Invert(const Matrix& src)
{
	if (src.identity)
	{
		Identity();
		return;
	}

	identity = false;	
	SetChanged(true);
    if ((src.data[0][3] == 0) && (src.data[1][3] == 0) && (src.data[2][3] == 0))
	{
        Down_triangle_Inverse_Xform(src.data, data);
        return;
    }
	else if ((src.data[3][0] == 0) && (src.data[3][1] == 0) && (src.data[3][2] == 0))
	{
        Up_triangle_Inverse_Xform(src.data, data);
        return;
    }
	else
	{
        Full_Inverse_Xform3(src.data, data);
        return;
    }

}

/*!
 * @fn void Matrix::Invert()
 *
 * Replaces this matrix with its inverse.
 *
 * @see Matrix::Transpose
 */
void Matrix::Invert()
{
Matrix tmp(*this);

	Invert(tmp);
}

Matrix &Matrix::operator=(const Matrix &src)
{
	Copy(src);
	return *this;
}

/***
 *
 * class Matrix override for SharedObj::Copy
 *
 ***/
bool Matrix::Copy(const SharedObj* src)
{
	const Matrix* t = (const Matrix*) src;

	ObjectLock dlock(this);
	ObjectLock slock(src);
	VX_ASSERT(t->IsClass(VX_Matrix));
	Copy(*t);
	return SharedObj::Copy(src);
}

void Matrix::Copy(const Matrix &src)
{
	const Matrix& a = (const Matrix&) src;

	if (identity == a.identity && identity)
		return;
	identity = a.identity;
	SetChanged(true);
	data[0][0]=a.data[0][0]; data[0][1]=a.data[0][1]; data[0][2]=a.data[0][2]; data[0][3]=a.data[0][3];
	data[1][0]=a.data[1][0]; data[1][1]=a.data[1][1]; data[1][2]=a.data[1][2]; data[1][3]=a.data[1][3];
	data[2][0]=a.data[2][0]; data[2][1]=a.data[2][1]; data[2][2]=a.data[2][2]; data[2][3]=a.data[2][3];
	data[3][0]=a.data[3][0]; data[3][1]=a.data[3][1]; data[3][2]=a.data[3][2]; data[3][3]=a.data[3][3];
}

/***
 *
 * class Matrix override for SharedObj::Do
 *	TRANS_Identity
 *	TRANS_SetData	gfloat[4][4]
 *
 ****/
bool Matrix::Do(Messenger& s, int op)
{
	switch (op)
	{
		case MATRIX_Identity:
		Identity();
		break;

		case MATRIX_SetMatrix:
		float mtx[16];
		s.Input((float*) mtx, 16); 
		SetMatrix(mtx);
		break;

		default:
		return SharedObj::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Matrix::DoNames[op - MATRIX_Identity]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * class Matrix override for SharedObj::Save
 *
 ****/
int Matrix::Save(Messenger& s, int opts) const
{
	int32 h = SharedObj::Save(s, opts);
	if (h > 0)
	{
		s << OP(VX_Matrix, MATRIX_SetMatrix) << h;
		s.Output(GetMatrix(), 16);
	}
	return h;
}



static void Full_Inverse_Xform3(const float b[4][4], float a[4][4])
{
    long indxc[4], indxr[4], ipiv[4];
    long i, icol = 0, irow = 0, j, ir, ic;
    float big, dum, pivinv, temp, bb;
    ipiv[0] = -1;
    ipiv[1] = -1;
    ipiv[2] = -1;
    ipiv[3] = -1;
    a[0][0] = b[0][0];
    a[1][0] = b[1][0];
    a[2][0] = b[2][0];
    a[3][0] = b[3][0];
    a[0][1] = b[0][1];
    a[1][1] = b[1][1];
    a[2][1] = b[2][1];
    a[3][1] = b[3][1];
    a[0][2] = b[0][2];
    a[1][2] = b[1][2];
    a[2][2] = b[2][2];
    a[3][2] = b[3][2];
    a[0][3] = b[0][3];
    a[1][3] = b[1][3];
    a[2][3] = b[2][3];
    a[3][3] = b[3][3];
    for (i = 0; i < 4; i++)
	{
        big = 0.0f;
        for (j = 0; j < 4; j++)
		{
            if (ipiv[j] != 0)
			{
                if (ipiv[0] == -1)
				{
                    if ((bb = (float) fabs(a[j][0])) > big)
					{
                        big = bb;
                        irow = j;
                        icol = 0;
                    }
                }
				else if (ipiv[1] > 0)
				{
					   return;
				}
				if (ipiv[2] == -1)
				{
					if ((bb = (float) fabs((float) a[j][2])) > big)
					{
						big = bb;
						irow = j;
						icol = 2;
					}
				}
				else if (ipiv[2] > 0)
				{
                    return;
                }
                if (ipiv[3] == -1)
				{
                    if ((bb = (float) fabs((float) a[j][3])) > big)
					{
                        big = bb;
                        irow = j;
                        icol = 3;
                    }
                }
				else if (ipiv[3] > 0)
				{
                    return;
                }
            }
        }
        ++(ipiv[icol]);
        if (irow != icol)
		{
            temp = a[irow][0];
            a[irow][0] = a[icol][0];
            a[icol][0] = temp;
            temp = a[irow][1];
            a[irow][1] = a[icol][1];
            a[icol][1] = temp;
            temp = a[irow][2];
            a[irow][2] = a[icol][2];
            a[icol][2] = temp;
            temp = a[irow][3];
            a[irow][3] = a[icol][3];
            a[icol][3] = temp;
        }
        indxr[i] = irow;
        indxc[i] = icol;
        if (a[icol][icol] == 0.0)
		{
            return;
        }
        pivinv = 1.0f / a[icol][icol];
        a[icol][icol] = 1.0f;
        a[icol][0] *= pivinv;
        a[icol][1] *= pivinv;
        a[icol][2] *= pivinv;
        a[icol][3] *= pivinv;
        if (icol != 0)
		{
            dum = a[0][icol];
            a[0][icol] = 0.0f;
            a[0][0] -= a[icol][0] * dum;
            a[0][1] -= a[icol][1] * dum;
            a[0][2] -= a[icol][2] * dum;
            a[0][3] -= a[icol][3] * dum;
        }
        if (icol != 1)
		{
            dum = a[1][icol];
            a[1][icol] = 0.0f;
            a[1][0] -= a[icol][0] * dum;
            a[1][1] -= a[icol][1] * dum;
            a[1][2] -= a[icol][2] * dum;
            a[1][3] -= a[icol][3] * dum;
        }
        if (icol != 2)
		{
            dum = a[2][icol];
            a[2][icol] = 0.0f;
            a[2][0] -= a[icol][0] * dum;
            a[2][1] -= a[icol][1] * dum;
            a[2][2] -= a[icol][2] * dum;
            a[2][3] -= a[icol][3] * dum;
        }
        if (icol != 3)
		{
            dum = a[3][icol];
            a[3][icol] = 0.0f;
            a[3][0] -= a[icol][0] * dum;
            a[3][1] -= a[icol][1] * dum;
            a[3][2] -= a[icol][2] * dum;
            a[3][3] -= a[icol][3] * dum;
        }
    }
    if (indxr[3] != indxc[3])
	{
        ir = indxr[3];
        ic = indxc[3];
        temp = a[0][ir];
        a[0][ir] = a[0][ic];
        a[0][ic] = temp;
        temp = a[1][ir];
        a[1][ir] = a[1][ic];
        a[1][ic] = temp;
        temp = a[2][ir];
        a[2][ir] = a[2][ic];
        a[2][ic] = temp;
        temp = a[3][ir];
        a[3][ir] = a[3][ic];
        a[3][ic] = temp;
    }
    if (indxr[2] != indxc[2])
	{
        ir = indxr[2];
        ic = indxc[2];
        temp = a[0][ir];
        a[0][ir] = a[0][ic];
        a[0][ic] = temp;
        temp = a[1][ir];
        a[1][ir] = a[1][ic];
        a[1][ic] = temp;
        temp = a[2][ir];
        a[2][ir] = a[2][ic];
        a[2][ic] = temp;
        temp = a[3][ir];
        a[3][ir] = a[3][ic];
        a[3][ic] = temp;
    }
    if (indxr[1] != indxc[1])
	{
        ir = indxr[1];
        ic = indxc[1];
        temp = a[0][ir];
        a[0][ir] = a[0][ic];
        a[0][ic] = temp;
        temp = a[1][ir];
        a[1][ir] = a[1][ic];
        a[1][ic] = temp;
        temp = a[2][ir];
        a[2][ir] = a[2][ic];
        a[2][ic] = temp;
        temp = a[3][ir];
        a[3][ir] = a[3][ic];
        a[3][ic] = temp;
    }
    if (indxr[0] != indxc[0])
	{
        ir = indxr[0];
        ic = indxc[0];
        temp = a[0][ir];
        a[0][ir] = a[0][ic];
        a[0][ic] = temp;
        temp = a[1][ir];
        a[1][ir] = a[1][ic];
        a[1][ic] = temp;
        temp = a[2][ir];
        a[2][ir] = a[2][ic];
        a[2][ic] = temp;
        temp = a[3][ir];
        a[3][ir] = a[3][ic];
        a[3][ic] = temp;
    }
}

static float det3x3(float a1, float a2, float a3, float b1, float b2, float b3, float c1, float c2, float c3)
{
        return (float) (a1 * (b2 * c3 - b3 * c2) - b1 * (a2 * c3 - a3 * c2) + c1 * (a2 * b3 - a3 * b2));
}

static void Down_triangle_Inverse_Xform(const float src[4][4], float dst[4][4])
{
    float det1, o33;
    float a1, a2, a3, b1, b2, b3, c1, c2, c3;
    a1 = src[0][0];
    b1 = src[0][1];
    c1 = src[0][2];
    a2 = src[1][0];
    b2 = src[1][1];
    c2 = src[1][2];
    a3 = src[2][0];
    b3 = src[2][1];
    c3 = src[2][2];
    det1 = det3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3);
    if (fabs(det1) < VX_SMALL_NUMBER) {
        return;
    } else {
        det1 = 1.0f / det1;
        o33 = 1.0f / src[3][3];
        dst[0][0] =  (b2 * c3 - c2 * b3) * det1;
        dst[1][0] = -(a2 * c3 - c2 * a3) * det1;
        dst[2][0] =  (a2 * b3 - b2 * a3) * det1;
        dst[0][1] = -(b1 * c3 - c1 * b3) * det1;
        dst[1][1] =  (a1 * c3 - c1 * a3) * det1;
        dst[2][1] = -(a1 * b3 - b1 * a3) * det1;
        dst[0][2] =  (b1 * c2 - c1 * b2) * det1;
        dst[1][2] = -(a1 * c2 - c1 * a2) * det1;
        dst[2][2] =  (a1 * b2 - b1 * a2) * det1;
        dst[3][0] = -(src[3][0] * dst[0][0] + src[3][1] * dst[1][0] + src[3][2] * dst[2][0]) * o33;
        dst[3][1] = -(src[3][0] * dst[0][1] + src[3][1] * dst[1][1] + src[3][2] * dst[2][1]) * o33;
        dst[3][2] = -(src[3][0] * dst[0][2] + src[3][1] * dst[1][2] + src[3][2] * dst[2][2]) * o33;
        dst[0][3] = 0.0f;
        dst[1][3] = 0.0f;
        dst[2][3] = 0.0f;
        dst[3][3] = o33;
    }
}

static void Up_triangle_Inverse_Xform(const float src[4][4], float dst[4][4])
{
    float det1, o33;
    float a1, a2, a3, b1, b2, b3, c1, c2, c3;
    a1 = src[0][0];
    b1 = src[0][1];
    c1 = src[0][2];
    a2 = src[1][0];
    b2 = src[1][1];
    c2 = src[1][2];
    a3 = src[2][0];
    b3 = src[2][1];
    c3 = src[2][2];
    det1 = det3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3);
    if (fabs(det1) < VX_SMALL_NUMBER) {
        return;
    } else {
        det1 = (float) (1.0f / det1);
        o33 = 1.0f / src[3][3];
        dst[0][0] =  (b2 * c3 - c2 * b3) * det1;
        dst[1][0] = -(a2 * c3 - c2 * a3) * det1;
        dst[2][0] =  (a2 * b3 - b2 * a3) * det1;
        dst[0][1] = -(b1 * c3 - c1 * b3) * det1;
        dst[1][1] =  (a1 * c3 - c1 * a3) * det1;
        dst[2][1] = -(a1 * b3 - b1 * a3) * det1;
        dst[0][2] =  (b1 * c2 - c1 * b2) * det1;
        dst[1][2] = -(a1 * c2 - c1 * a2) * det1;
        dst[2][2] =  (a1 * b2 - b1 * a2) * det1;
        dst[0][3] = -(src[0][3] * dst[0][0] + src[1][3] * dst[0][1] + src[2][3] * dst[0][2]) * o33;
        dst[1][3] = -(src[0][3] * dst[1][0] + src[1][3] * dst[1][1] + src[2][3] * dst[1][2]) * o33;
        dst[2][3] = -(src[0][3] * dst[2][0] + src[1][3] * dst[2][1] + src[2][3] * dst[2][2]) * o33;
        dst[3][0] = 0.0f;
        dst[3][1] = 0.0f;
        dst[3][2] = 0.0f;
        dst[3][3] = o33;
    }
}

}	// end Vixen