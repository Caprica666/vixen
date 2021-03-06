/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class VertexArray extends VertexPool {
  private long swigCPtr;

  public VertexArray(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.VertexArray_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(VertexArray obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_VertexArray(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public VertexArray(int style, long size) {
    this(VixenLibJNI.new_VertexArray__SWIG_0(style, size), true);
  }

  public VertexArray(int style) {
    this(VixenLibJNI.new_VertexArray__SWIG_1(style), true);
  }

  public VertexArray() {
    this(VixenLibJNI.new_VertexArray__SWIG_2(), true);
  }

  public VertexArray(String layout_desc, long size) {
    this(VixenLibJNI.new_VertexArray__SWIG_3(layout_desc, size), true);
  }

  public VertexArray(String layout_desc) {
    this(VixenLibJNI.new_VertexArray__SWIG_4(layout_desc), true);
  }

  public VertexArray(VertexArray arg0) {
    this(VixenLibJNI.new_VertexArray__SWIG_5(VertexArray.getCPtr(arg0), arg0), true);
  }

  public boolean SetMaxVtx(long size) {
    return VixenLibJNI.VertexArray_SetMaxVtx(swigCPtr, this, size);
  }

  public boolean SetNumVtx(long size) {
    return VixenLibJNI.VertexArray_SetNumVtx(swigCPtr, this, size);
  }

  public long AddVertices(SWIGTYPE_p_float floatArray, long size) {
    return VixenLibJNI.VertexArray_AddVertices(swigCPtr, this, SWIGTYPE_p_float.getCPtr(floatArray), size);
  }

  public SWIGTYPE_p_float PadVertices(SWIGTYPE_p_float floatArray, long size, long srcstride) {
    long cPtr = VixenLibJNI.VertexArray_PadVertices(swigCPtr, this, SWIGTYPE_p_float.getCPtr(floatArray), size, srcstride);
    return (cPtr == 0) ? null : new SWIGTYPE_p_float(cPtr, false);
  }

  public boolean Copy(SharedObj arg0) {
    return VixenLibJNI.VertexArray_Copy(swigCPtr, this, SharedObj.getCPtr(arg0), arg0);
  }

  public static void setVertexAlloc(BaseObj value) {
    VixenLibJNI.VertexArray_VertexAlloc_set(BaseObj.getCPtr(value), value);
  }

  public static BaseObj getVertexAlloc() {
    long cPtr = VixenLibJNI.VertexArray_VertexAlloc_get();
    return (cPtr == 0) ? null : new BaseObj(cPtr, false);
  }

}
