#pragma once
/*
 * Interoperability classes for Vixen and Havok math objects
 */
namespace Vixen
{
	struct Vec3H : public Vec3
	{
		Vec3H() : Vec3() { }
		Vec3H(const hkVector4& src)			{ *this = src; }
		Vec3H(float x, float y, float z) : Vec3(x, y, z) { }
		Vec3H(const float* floatArray) : Vec3(floatArray) { }
		Vec3H(const Vec3& src) : Vec3(src) { }
		operator hkVector4	()				{ return hkVector4(x, y, z); }

		Vec3H& operator=(const Vec3& src)	{ Vec3::operator=(src); return *this; }
		Vec3H& operator=(const hkVector4& src)
		{
			x = src(0);
			y = src(1);
			z = src(2);
			return *this;
		}
	};

	struct Vec4H : public Vec4
	{
		Vec4H() : Vec4() { }
		Vec4H(const hkVector4& src)	{ *this = src; }
		Vec4H(float x, float y, float z, float w) : Vec4(x, y, z, w) { }
		Vec4H(const float* floatArray) : Vec4(floatArray) { }
		Vec4H(const Vec4& src) : Vec4(src) { }
		operator hkVector4	()		{ return hkVector4(x, y, z); }

		Vec4H& operator=(const Vec4& src)	{ Vec4::operator=(src); return *this; }
		Vec4H& operator=(const hkVector4& src)
		{
			x = src(0);
			y = src(1);
			z = src(2);
			w = src(3);
			return *this;
		}
	};

	struct QuatH : public Quat
	{
		QuatH() : Quat(0.0f, 0.0f, 0.0f, 1.0f) { }
		QuatH(const Quat& src) : Quat(src) { Normalize(); }
		QuatH(const hkQuaternion& src)	{ *this = src; }
		QuatH(float x, float y, float z, float w) : Quat(x, y, z, w) { }
		QuatH& operator=(const Quat& src)	{ Quat::operator=(src); return *this; }

		QuatH(const hkRotation& src)
		{
			hkQuaternion q;
			q.setAndNormalize(src);
			x = q(0);
			y = q(1);
			z = q(2);
			w = q(3);
		}

		QuatH& operator=(const hkQuaternion& src)
		{
			x = src(0);
			y = src(1);
			z = src(2);
			w = src(3);
			return *this;
		}

		operator hkQuaternion()
		{
			hkQuaternion q(x, y, z, w);
			q.normalize();
			return q;
		}
	};

	class MatrixH : public Matrix
	{
	public:
		MatrixH(const Matrix& mtx) : Matrix(mtx) { }
		MatrixH() : Matrix() { }
		MatrixH(const hkQuaternion& src) : Matrix(QuatH(src)) { }
		MatrixH(const hkRotation& src) : Matrix(QuatH(src)) { }
		MatrixH(const hkTransform& src)		{ *this = src; }
		MatrixH(const hkQsTransform& src)	{ *this = src; }
		MatrixH(const hkMatrix4& src)		{ *this = src; }
		MatrixH& operator=(const Matrix& src)		{ Matrix::operator=(src); return *this; }
		MatrixH& operator=(const hkQuaternion& src) { Set(QuatH(src)); return *this; }
		MatrixH& operator=(const hkRotation& src)	{ Set(QuatH(src)); return *this; }

		MatrixH& operator=(const hkTransform& src)
		{
			HK_ALIGN16(hkReal transdata[16]);
			hkMatrix4	tmp;
			tmp.set(src);
			tmp.get4x4RowMajor((hkFloat32*) transdata);
			SetMatrix(transdata);
			return *this;
		}

		MatrixH& operator=(const hkMatrix4& src)
		{
			HK_ALIGN16(hkReal transdata[16]);
			src.get4x4RowMajor((hkFloat32*) transdata);
			SetMatrix(transdata);
			return *this;
		}

		MatrixH& operator=(const hkQsTransform& src)
		{
			hkTransform hkt;
			HK_ALIGN16(hkFloat32 transdata[16]);
			
			src.copyToTransformNoScale(hkt); 
			hkt.get4x4ColumnMajor((hkFloat32*) transdata);
			SetMatrix(transdata);
			Transpose();
			return *this;
		}

		operator hkQsTransform()
		{
			HK_ALIGN16(hkFloat32 transdata[16]);
			hkMatrix4	tmp;
			hkQsTransform hkt;

			tmp.set4x4RowMajor((hkFloat32*) GetMatrix());
			tmp.get4x4ColumnMajor(transdata);
			hkt.set4x4ColumnMajor(transdata);
			return hkt;
		}

		operator hkMatrix4()
		{
			hkMatrix4 hkt;

			hkt.set4x4RowMajor((hkFloat32*) GetMatrix());
			return hkt;
		}
	};

}	// end Vixen
