/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class Ray {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  public Ray(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  public static long getCPtr(Ray obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_Ray(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public void setStart(Vec3 value) {
    VixenLibJNI.Ray_start_set(swigCPtr, this, Vec3.getCPtr(value), value);
  }

  public Vec3 getStart() {
    return new Vec3(VixenLibJNI.Ray_start_get(swigCPtr, this), false);
  }

  public void setEnd(Vec3 value) {
    VixenLibJNI.Ray_end_set(swigCPtr, this, Vec3.getCPtr(value), value);
  }

  public Vec3 getEnd() {
    return new Vec3(VixenLibJNI.Ray_end_get(swigCPtr, this), false);
  }

  public void setDirection(Vec3 value) {
    VixenLibJNI.Ray_direction_set(swigCPtr, this, Vec3.getCPtr(value), value);
  }

  public Vec3 getDirection() {
    return new Vec3(VixenLibJNI.Ray_direction_get(swigCPtr, this), false);
  }

  public void setLength(float value) {
    VixenLibJNI.Ray_length_set(swigCPtr, this, value);
  }

  public float getLength() {
    return VixenLibJNI.Ray_length_get(swigCPtr, this);
  }

  public Ray() {
    this(VixenLibJNI.new_Ray__SWIG_0(), true);
  }

  public Ray(Vec3 start, Vec3 direction, float length) {
    this(VixenLibJNI.new_Ray__SWIG_1(Vec3.getCPtr(start), start, Vec3.getCPtr(direction), direction, length), true);
  }

  public Ray(Vec3 start, Vec3 end) {
    this(VixenLibJNI.new_Ray__SWIG_2(Vec3.getCPtr(start), start, Vec3.getCPtr(end), end), true);
  }

  public void Set(Vec3 start, Vec3 end) {
    VixenLibJNI.Ray_Set(swigCPtr, this, Vec3.getCPtr(start), start, Vec3.getCPtr(end), end);
  }

}
