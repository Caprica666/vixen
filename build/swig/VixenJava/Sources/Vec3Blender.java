/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class Vec3Blender extends Evaluator {
  private long swigCPtr;

  public Vec3Blender(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.Vec3Blender_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(Vec3Blender obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_Vec3Blender(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public Vec3Blender() {
    this(VixenLibJNI.new_Vec3Blender(), true);
  }

  public SWIGTYPE_p_float GetXInput() {
    long cPtr = VixenLibJNI.Vec3Blender_GetXInput(swigCPtr, this);
    return (cPtr == 0) ? null : new SWIGTYPE_p_float(cPtr, false);
  }

  public SWIGTYPE_p_float GetYInput() {
    long cPtr = VixenLibJNI.Vec3Blender_GetYInput(swigCPtr, this);
    return (cPtr == 0) ? null : new SWIGTYPE_p_float(cPtr, false);
  }

  public SWIGTYPE_p_float GetZInput() {
    long cPtr = VixenLibJNI.Vec3Blender_GetZInput(swigCPtr, this);
    return (cPtr == 0) ? null : new SWIGTYPE_p_float(cPtr, false);
  }

  public void SetXInput(SWIGTYPE_p_float v) {
    VixenLibJNI.Vec3Blender_SetXInput(swigCPtr, this, SWIGTYPE_p_float.getCPtr(v));
  }

  public void SetYInput(SWIGTYPE_p_float v) {
    VixenLibJNI.Vec3Blender_SetYInput(swigCPtr, this, SWIGTYPE_p_float.getCPtr(v));
  }

  public void SetZInput(SWIGTYPE_p_float v) {
    VixenLibJNI.Vec3Blender_SetZInput(swigCPtr, this, SWIGTYPE_p_float.getCPtr(v));
  }

}
