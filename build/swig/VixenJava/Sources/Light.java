/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class Light extends Model {
  private long swigCPtr;

  public Light(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.Light_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(Light obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_Light(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public Light(String layout_desc, String shadername) {
    this(VixenLibJNI.new_Light__SWIG_0(layout_desc, shadername), true);
  }

  public Light(String layout_desc) {
    this(VixenLibJNI.new_Light__SWIG_1(layout_desc), true);
  }

  public Light() {
    this(VixenLibJNI.new_Light__SWIG_2(), true);
  }

  public void SetColor(Col4 arg0) {
    VixenLibJNI.Light_SetColor(swigCPtr, this, Col4.getCPtr(arg0), arg0);
  }

  public Col4 GetColor() {
    return new Col4(VixenLibJNI.Light_GetColor(swigCPtr, this), false);
  }

  public void SetRadius(float arg0) {
    VixenLibJNI.Light_SetRadius(swigCPtr, this, arg0);
  }

  public float GetRadius() {
    return VixenLibJNI.Light_GetRadius(swigCPtr, this);
  }

  public void SetDecay(int arg0) {
    VixenLibJNI.Light_SetDecay(swigCPtr, this, arg0);
  }

  public int GetDecay() {
    return VixenLibJNI.Light_GetDecay(swigCPtr, this);
  }

  public void Attach(Scene arg0) {
    VixenLibJNI.Light_Attach(swigCPtr, this, Scene.getCPtr(arg0), arg0);
  }

  public DeviceBuffer GetDataBuffer() {
    long cPtr = VixenLibJNI.Light_GetDataBuffer__SWIG_0(swigCPtr, this);
    return (cPtr == 0) ? null : new DeviceBuffer(cPtr, false);
  }

  public boolean Copy(SharedObj src) {
    return VixenLibJNI.Light_Copy(swigCPtr, this, SharedObj.getCPtr(src), src);
  }

  public void SetActive(boolean active) {
    VixenLibJNI.Light_SetActive(swigCPtr, this, active);
  }

  public Vec3 GetCenter(int arg0) {
    return new Vec3(VixenLibJNI.Light_GetCenter__SWIG_0(swigCPtr, this, arg0), true);
  }

  public Vec3 GetCenter() {
    return new Vec3(VixenLibJNI.Light_GetCenter__SWIG_1(swigCPtr, this), true);
  }

  public boolean GetCenter(Vec3 arg0, int arg1) {
    return VixenLibJNI.Light_GetCenter__SWIG_2(swigCPtr, this, Vec3.getCPtr(arg0), arg0, arg1);
  }

  public boolean GetCenter(Vec3 arg0) {
    return VixenLibJNI.Light_GetCenter__SWIG_3(swigCPtr, this, Vec3.getCPtr(arg0), arg0);
  }

  public final static int NONE = VixenLibJNI.Light_NONE_get();
  public final static int INVERSE = VixenLibJNI.Light_INVERSE_get();
  public final static int INVERSE_SQUARE = VixenLibJNI.Light_INVERSE_SQUARE_get();

}
