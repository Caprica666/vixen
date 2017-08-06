/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class Box3 {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  public Box3(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  public static long getCPtr(Box3 obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_Box3(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public void setMin(Vec3 value) {
    VixenLibJNI.Box3_min_set(swigCPtr, this, Vec3.getCPtr(value), value);
  }

  public Vec3 getMin() {
    return new Vec3(VixenLibJNI.Box3_min_get(swigCPtr, this), false);
  }

  public void setMax(Vec3 value) {
    VixenLibJNI.Box3_max_set(swigCPtr, this, Vec3.getCPtr(value), value);
  }

  public Vec3 getMax() {
    return new Vec3(VixenLibJNI.Box3_max_get(swigCPtr, this), false);
  }

  public Box3() {
    this(VixenLibJNI.new_Box3__SWIG_0(), true);
  }

  public Box3(Box3 arg0) {
    this(VixenLibJNI.new_Box3__SWIG_1(Box3.getCPtr(arg0), arg0), true);
  }

  public Box3(Sphere arg0) {
    this(VixenLibJNI.new_Box3__SWIG_2(Sphere.getCPtr(arg0), arg0), true);
  }

  public Box3(Vec3 arg0, Vec3 arg1) {
    this(VixenLibJNI.new_Box3__SWIG_3(Vec3.getCPtr(arg0), arg0, Vec3.getCPtr(arg1), arg1), true);
  }

  public Box3(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax) {
    this(VixenLibJNI.new_Box3__SWIG_4(xmin, ymin, zmin, xmax, ymax, zmax), true);
  }

  public void Set(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax) {
    VixenLibJNI.Box3_Set__SWIG_0(swigCPtr, this, xmin, ymin, zmin, xmax, ymax, zmax);
  }

  public void Set(Vec3 arg0, Vec3 arg1) {
    VixenLibJNI.Box3_Set__SWIG_1(swigCPtr, this, Vec3.getCPtr(arg0), arg0, Vec3.getCPtr(arg1), arg1);
  }

  public void Empty() {
    VixenLibJNI.Box3_Empty(swigCPtr, this);
  }

  public Box3 Assign(Box3 arg0) {
    return new Box3(VixenLibJNI.Box3_Assign__SWIG_0(swigCPtr, this, Box3.getCPtr(arg0), arg0), false);
  }

  public Box3 Assign(Sphere arg0) {
    return new Box3(VixenLibJNI.Box3_Assign__SWIG_1(swigCPtr, this, Sphere.getCPtr(arg0), arg0), false);
  }

  public Box3 Mul(Matrix arg0) {
    return new Box3(VixenLibJNI.Box3_Mul(swigCPtr, this, Matrix.getCPtr(arg0), arg0), false);
  }

  public boolean Equal(Box3 arg0) {
    return VixenLibJNI.Box3_Equal(swigCPtr, this, Box3.getCPtr(arg0), arg0);
  }

  public boolean NotEqual(Box3 arg0) {
    return VixenLibJNI.Box3_NotEqual(swigCPtr, this, Box3.getCPtr(arg0), arg0);
  }

  public void Normalize() {
    VixenLibJNI.Box3_Normalize(swigCPtr, this);
  }

  public float Width() {
    return VixenLibJNI.Box3_Width(swigCPtr, this);
  }

  public float Height() {
    return VixenLibJNI.Box3_Height(swigCPtr, this);
  }

  public float Depth() {
    return VixenLibJNI.Box3_Depth(swigCPtr, this);
  }

  public Vec3 Center() {
    return new Vec3(VixenLibJNI.Box3_Center(swigCPtr, this), true);
  }

  public boolean IsEmpty() {
    return VixenLibJNI.Box3_IsEmpty(swigCPtr, this);
  }

  public boolean IsNull() {
    return VixenLibJNI.Box3_IsNull(swigCPtr, this);
  }

  public boolean Contains(Vec3 arg0) {
    return VixenLibJNI.Box3_Contains(swigCPtr, this, Vec3.getCPtr(arg0), arg0);
  }

  public void Around(Vec3 arg0, Vec3 arg1) {
    VixenLibJNI.Box3_Around(swigCPtr, this, Vec3.getCPtr(arg0), arg0, Vec3.getCPtr(arg1), arg1);
  }

  public void Extend(Box3 arg0) {
    VixenLibJNI.Box3_Extend__SWIG_0(swigCPtr, this, Box3.getCPtr(arg0), arg0);
  }

  public void Extend(Vec3 arg0) {
    VixenLibJNI.Box3_Extend__SWIG_1(swigCPtr, this, Vec3.getCPtr(arg0), arg0);
  }

}
