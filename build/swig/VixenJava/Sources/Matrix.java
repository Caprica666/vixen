/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class Matrix extends SharedObj {
  private long swigCPtr;

  public Matrix(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.Matrix_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(Matrix obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_Matrix(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public Matrix() {
    this(VixenLibJNI.new_Matrix__SWIG_0(), true);
  }

  public Matrix(Matrix rhs) {
    this(VixenLibJNI.new_Matrix__SWIG_1(Matrix.getCPtr(rhs), rhs), true);
  }

  public Matrix(Quat arg0) {
    this(VixenLibJNI.new_Matrix__SWIG_2(Quat.getCPtr(arg0), arg0), true);
  }

  public Matrix(SWIGTYPE_p_float floatArray) {
    this(VixenLibJNI.new_Matrix__SWIG_3(SWIGTYPE_p_float.getCPtr(floatArray)), true);
  }

  public void Identity() {
    VixenLibJNI.Matrix_Identity(swigCPtr, this);
  }

  public void Zero() {
    VixenLibJNI.Matrix_Zero(swigCPtr, this);
  }

  public void ScaleMatrix(Vec3 arg0) {
    VixenLibJNI.Matrix_ScaleMatrix(swigCPtr, this, Vec3.getCPtr(arg0), arg0);
  }

  public void TranslationMatrix(Vec3 arg0) {
    VixenLibJNI.Matrix_TranslationMatrix(swigCPtr, this, Vec3.getCPtr(arg0), arg0);
  }

  public void Translate(Vec3 arg0) {
    VixenLibJNI.Matrix_Translate__SWIG_0(swigCPtr, this, Vec3.getCPtr(arg0), arg0);
  }

  public void Translate(float x, float y, float z) {
    VixenLibJNI.Matrix_Translate__SWIG_1(swigCPtr, this, x, y, z);
  }

  public void Scale(Vec3 arg0) {
    VixenLibJNI.Matrix_Scale__SWIG_0(swigCPtr, this, Vec3.getCPtr(arg0), arg0);
  }

  public void Scale(float x, float y, float z) {
    VixenLibJNI.Matrix_Scale__SWIG_1(swigCPtr, this, x, y, z);
  }

  public void LookAt(Vec3 v, float twist) {
    VixenLibJNI.Matrix_LookAt(swigCPtr, this, Vec3.getCPtr(v), v, twist);
  }

  public void Rotate(Vec3 axis, float angle) {
    VixenLibJNI.Matrix_Rotate__SWIG_0(swigCPtr, this, Vec3.getCPtr(axis), axis, angle);
  }

  public void Rotate(Quat q) {
    VixenLibJNI.Matrix_Rotate__SWIG_1(swigCPtr, this, Quat.getCPtr(q), q);
  }

  public void Set(Quat arg0) {
    VixenLibJNI.Matrix_Set__SWIG_0(swigCPtr, this, Quat.getCPtr(arg0), arg0);
  }

  public void RotationMatrix(Quat arg0) {
    VixenLibJNI.Matrix_RotationMatrix__SWIG_0(swigCPtr, this, Quat.getCPtr(arg0), arg0);
  }

  public void RotationMatrix(Vec3 v, float angle) {
    VixenLibJNI.Matrix_RotationMatrix__SWIG_1(swigCPtr, this, Vec3.getCPtr(v), v, angle);
  }

  public void XAngleMatrix(float angle) {
    VixenLibJNI.Matrix_XAngleMatrix(swigCPtr, this, angle);
  }

  public void YAngleMatrix(float angle) {
    VixenLibJNI.Matrix_YAngleMatrix(swigCPtr, this, angle);
  }

  public void ZAngleMatrix(float angle) {
    VixenLibJNI.Matrix_ZAngleMatrix(swigCPtr, this, angle);
  }

  public static Matrix GetIdentity() {
    long cPtr = VixenLibJNI.Matrix_GetIdentity();
    return (cPtr == 0) ? null : new Matrix(cPtr, false);
  }

  public void SetMatrix(SWIGTYPE_p_float floatArray) {
    VixenLibJNI.Matrix_SetMatrix(swigCPtr, this, SWIGTYPE_p_float.getCPtr(floatArray));
  }

  public Vec4 GetRotationAxis() {
    return new Vec4(VixenLibJNI.Matrix_GetRotationAxis(swigCPtr, this), true);
  }

  public void Set(int i1, int i2, float arg2) {
    VixenLibJNI.Matrix_Set__SWIG_1(swigCPtr, this, i1, i2, arg2);
  }

  public float Get(int i1, int i2) {
    return VixenLibJNI.Matrix_Get(swigCPtr, this, i1, i2);
  }

  public boolean IsIdentity() {
    return VixenLibJNI.Matrix_IsIdentity(swigCPtr, this);
  }

  public void Copy(Matrix src) {
    VixenLibJNI.Matrix_Copy__SWIG_0(swigCPtr, this, Matrix.getCPtr(src), src);
  }

  public boolean Copy(SharedObj arg0) {
    return VixenLibJNI.Matrix_Copy__SWIG_1(swigCPtr, this, SharedObj.getCPtr(arg0), arg0);
  }

  public void Touch() {
    VixenLibJNI.Matrix_Touch(swigCPtr, this);
  }

  public void GetTranslation(Vec3 arg0) {
    VixenLibJNI.Matrix_GetTranslation(swigCPtr, this, Vec3.getCPtr(arg0), arg0);
  }

  public void SetTranslation(Vec3 arg0) {
    VixenLibJNI.Matrix_SetTranslation(swigCPtr, this, Vec3.getCPtr(arg0), arg0);
  }

  public void GetScale(Vec3 arg0) {
    VixenLibJNI.Matrix_GetScale(swigCPtr, this, Vec3.getCPtr(arg0), arg0);
  }

  public void GetRotation(Vec3 OUTPUT) {
    VixenLibJNI.Matrix_GetRotation(swigCPtr, this, Vec3.getCPtr(OUTPUT), OUTPUT);
  }

  public void PreMul(Matrix arg0) {
    VixenLibJNI.Matrix_PreMul(swigCPtr, this, Matrix.getCPtr(arg0), arg0);
  }

  public void PostMul(Matrix arg0) {
    VixenLibJNI.Matrix_PostMul(swigCPtr, this, Matrix.getCPtr(arg0), arg0);
  }

  public void Multiply(Matrix arg0, Matrix arg1) {
    VixenLibJNI.Matrix_Multiply(swigCPtr, this, Matrix.getCPtr(arg0), arg0, Matrix.getCPtr(arg1), arg1);
  }

  public void Transpose() {
    VixenLibJNI.Matrix_Transpose(swigCPtr, this);
  }

  public void Invert(Matrix src) {
    VixenLibJNI.Matrix_Invert__SWIG_0(swigCPtr, this, Matrix.getCPtr(src), src);
  }

  public void Invert() {
    VixenLibJNI.Matrix_Invert__SWIG_1(swigCPtr, this);
  }

  public void Transform(Vec4 src, Vec4 dst) {
    VixenLibJNI.Matrix_Transform__SWIG_0(swigCPtr, this, Vec4.getCPtr(src), src, Vec4.getCPtr(dst), dst);
  }

  public void Transform(Vec3 src, Vec4 dst) {
    VixenLibJNI.Matrix_Transform__SWIG_1(swigCPtr, this, Vec3.getCPtr(src), src, Vec4.getCPtr(dst), dst);
  }

  public void Transform(Vec3 src, Vec3 dst) {
    VixenLibJNI.Matrix_Transform__SWIG_2(swigCPtr, this, Vec3.getCPtr(src), src, Vec3.getCPtr(dst), dst);
  }

  public void TransformVector(Vec3 src, Vec3 dst) {
    VixenLibJNI.Matrix_TransformVector(swigCPtr, this, Vec3.getCPtr(src), src, Vec3.getCPtr(dst), dst);
  }

  public void Transform(Sphere src, Sphere dst) {
    VixenLibJNI.Matrix_Transform__SWIG_3(swigCPtr, this, Sphere.getCPtr(src), src, Sphere.getCPtr(dst), dst);
  }

}
