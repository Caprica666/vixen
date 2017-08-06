/*!
 * @file vxmatrix.h
 * @brief 4x4 matrix class for 3D graphics transformations.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxmath.h
 */
#pragma once

namespace Vixen {

class Sphere;

/*!
 * @class Matrix
 * @brief 4x4 Matrix transformation class that supports automation.
 *
 * @ingroup vixen
 * @see Vec3 Vec4 Quat
 */
class Matrix : public SharedObj
{
public:
	friend class World3D;							// for identity matrix init/clean

	VX_DECLARE_CLASS(Matrix);

//! @name	Matrix Initialization
//!@{
	Matrix();										//!< Construct identity matrix.
	Matrix(const Matrix& rhs);						//!< Construct matrix from another matrix.
	Matrix(const Quat&);							//!< Construct rotation matrix from quaternion.

	Matrix(const float* floatArray);				//!< Construct matrix from floating array.
	void	Identity();								//!< Make identity matrix.
	void	Zero();									//!< Zero this matrix.
	void	ScaleMatrix(const Vec3 &);				//!< Make scaling matrix.
	void	TranslationMatrix(const Vec3&);			//!< Make translation matrix.
	void	Translate(const Vec3&);					//!< Concatenate translation matrix.
	void	Translate(float x, float y, float z);	//!< Concatenate translation matrix.
	void	Scale(const Vec3&);						//!< Concatenate scaling matrix.
	void	Scale(float x, float y, float z);		//!< Concatenate scaling matrix.
	void	LookAt(const Vec3& v, float twist);		//!< Make matrix which looks at a point.
	void	Rotate(const Vec3& axis, float angle);	//!< Concatenate rotation matrix from axis and angle
	void	Rotate(const Quat& q);					//!< Concatentate quaternion rotation.
	void	Set(const Quat&);						//!< Make rotation matrix from quaternion
	void	RotationMatrix(const Quat&);			//!< Make rotation matrix from quaternion
	void	RotationMatrix(const Vec3* vecs);		//!< Make rotation matrix from three vectors
	void	RotationMatrix(const Vec3& v, float angle); //!< Make rotation matrix from axis and angle
	void	XAngleMatrix(float angle);
	void	YAngleMatrix(float angle);
	void	ZAngleMatrix(float angle);
	static const Matrix*	GetIdentity();			//!< Return the identity matrix, always the same one

//!@}

//! @name Element Access
//!@{
	const float*	GetMatrix() const;			//!< Return pointer to matrix element data.
	void	SetMatrix(const float* floatArray);	//!< Set matrix element data from float array.

	Vec4	GetRotationAxis() const;			//!< Get rotation axis and angle.
	void 	Set(int i1, int i2, float);			//!< Retrieve matrix element based on indices.
	float	Get(int i1, int i2) const;			//!< Update matrix element based on indices.
	bool	IsIdentity() const					//!< Check for identity matrix
			{ return identity; }
	void	Copy(const Matrix &src);			//!< Copy source matrix into this one.
	Matrix& operator=(const Matrix &src);		//!< Copy source matrix into this one.
	bool	Copy(const SharedObj*);				//!< Copy source matrix object into this one.
	void	Touch();							//!< Indicate matrix has been updated.
	void	GetTranslation(Vec3&) const;		//!< Get translation factors
	void	SetTranslation(const Vec3&);		//!< Set translation factors
	void	GetScale(Vec3&) const;				//!< Get scale factors from matrix.
	void	GetRotation(Vec3* OUTPUT) const;	//!< Get rotation vectors from matrix.
//!@}

//! @name Matrix Arithmetic
//!@{
	Matrix& operator+=(const Matrix&);				//!< Matrix addition.
	Matrix& operator-=(const Matrix&);				//!< Matrix subtraction.
	Matrix& operator*=(const Matrix&);				//!< Postmultiplication.
	bool	operator==(const Matrix&) const;		//!< Compare matrices for equality.
	bool	operator!=(const Matrix&) const;		//!< Compare matrices for inequality.
	void	PreMul(const Matrix&);					//!< Premultiplication.
	void	PostMul(const Matrix&);					//!< Postmultiplication.
	void	Multiply(const Matrix&, const Matrix&);	//! Multiplication
	void	Transpose();							//!< Transposition.
	void	Invert(const Matrix &src);				//!< Invert source matrix.
	void	Invert();								//!< Invert this matrix
//!@}

//! @name Transformation
//!@{
//! Transform homogeneous vector by 4x4 matrix
	void	Transform(const Vec4 &src, Vec4 &dst) const;
//! Transform 3D point by 4x4 matrix, produce homogeneous vector
	void	Transform(const Vec3 &src, Vec4 &dst) const;
//! Transform 3D point by 3x3 and translation, produce 3D point
	void	Transform(const Vec3 &src, Vec3 &dst) const;
//! Transform 3D vector by 3x3 matrix, produce 3D vector
	void	TransformVector(const Vec3 &src, Vec3 &dst) const;
//! Transform sphere by matrix
	void	Transform(const Sphere& src, Sphere &dst) const;
//!@}


//	SharedObj Overrides
	virtual bool	Do(Messenger& s, int op);
	virtual int		Save(Messenger&, int) const;
	virtual DebugOut& Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;
	
	/*
	 * Matix::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		MATRIX_Identity = SharedObj::OBJ_NextOp,
		MATRIX_SetMatrix,
		MATRIX_Inverse,
		MATRIX_Transpose,
		MATRIX_PostMul,
		MATRIX_PreMul,
		MATRIX_Add,
		MATRIX_Subtract,
		MATRIX_Translate,
		MATRIX_Scale,
		MATRIX_Rotate,
		MATRIX_Mul,
		MATRIX_LookAt,
		MATRIX_NextOp = SharedObj::OBJ_NextOp + 20,
	};

protected:
	static	Matrix* s_IdentityMatrix;	//!< Pre-constructed identity matrix
	float	ALIGN16	data[4][4];
	bool	identity;
};

} // end Vixen