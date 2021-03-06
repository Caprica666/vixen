/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class Geometry extends SharedObj {
  private long swigCPtr;

  public Geometry(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.Geometry_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(Geometry obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_Geometry(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public Geometry() {
    this(VixenLibJNI.new_Geometry__SWIG_0(), true);
  }

  public Geometry(Geometry arg0) {
    this(VixenLibJNI.new_Geometry__SWIG_1(Geometry.getCPtr(arg0), arg0), true);
  }

  public void Touch() {
    VixenLibJNI.Geometry_Touch(swigCPtr, this);
  }

  public void Empty() {
    VixenLibJNI.Geometry_Empty(swigCPtr, this);
  }

  public long GetNumFaces() {
    return VixenLibJNI.Geometry_GetNumFaces(swigCPtr, this);
  }

  public long GetNumVtx() {
    return VixenLibJNI.Geometry_GetNumVtx(swigCPtr, this);
  }

  public boolean GetBound(Box3 arg0) {
    return VixenLibJNI.Geometry_GetBound(swigCPtr, this, Box3.getCPtr(arg0), arg0);
  }

  public Geometry Mul(Matrix arg0) {
    return new Geometry(VixenLibJNI.Geometry_Mul(swigCPtr, this, Matrix.getCPtr(arg0), arg0), false);
  }

}
