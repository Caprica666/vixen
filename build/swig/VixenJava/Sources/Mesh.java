/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class Mesh extends Geometry {
  private long swigCPtr;

  public Mesh(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.Mesh_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(Mesh obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_Mesh(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public Mesh() {
    this(VixenLibJNI.new_Mesh__SWIG_0(), true);
  }

  public Mesh(int style, long maxvtx) {
    this(VixenLibJNI.new_Mesh__SWIG_1(style, maxvtx), true);
  }

  public Mesh(int style) {
    this(VixenLibJNI.new_Mesh__SWIG_2(style), true);
  }

  public Mesh(String layout_desc, long maxvtx) {
    this(VixenLibJNI.new_Mesh__SWIG_3(layout_desc, maxvtx), true);
  }

  public Mesh(String layout_desc) {
    this(VixenLibJNI.new_Mesh__SWIG_4(layout_desc), true);
  }

  public Mesh(Mesh arg0) {
    this(VixenLibJNI.new_Mesh__SWIG_5(Mesh.getCPtr(arg0), arg0), true);
  }

  public VertexArray GetVertices() 
	{
		jlong ptr = VixenLibJNI.Mesh_GetVertices(swigCPtr, this);
		return (VertexArray)  SharedObj.MakeObject(ptr, true);
	}


  public IntArray GetIndices() {
    long cPtr = VixenLibJNI.Mesh_GetIndices(swigCPtr, this);
    return (cPtr == 0) ? null : new IntArray(cPtr, false);
  }

  public void SetIndices(IntArray arg0) {
    VixenLibJNI.Mesh_SetIndices(swigCPtr, this, IntArray.getCPtr(arg0), arg0);
  }

  public void SetVertices(VertexArray arg0) {
    VixenLibJNI.Mesh_SetVertices(swigCPtr, this, VertexArray.getCPtr(arg0), arg0);
  }

  public int GetStyle() {
    return VixenLibJNI.Mesh_GetStyle(swigCPtr, this);
  }

  public int GetVtxSize() {
    return VixenLibJNI.Mesh_GetVtxSize(swigCPtr, this);
  }

  public long GetNumVtx() {
    return VixenLibJNI.Mesh_GetNumVtx(swigCPtr, this);
  }

  public boolean SetNumVtx(long arg0) {
    return VixenLibJNI.Mesh_SetNumVtx(swigCPtr, this, arg0);
  }

  public boolean SetMaxVtx(long arg0) {
    return VixenLibJNI.Mesh_SetMaxVtx(swigCPtr, this, arg0);
  }

  public long GetNumIdx() {
    return VixenLibJNI.Mesh_GetNumIdx(swigCPtr, this);
  }

  public long GetIndex(long index) {
    return VixenLibJNI.Mesh_GetIndex(swigCPtr, this, index);
  }

  public boolean SetIndex(long index, long v) {
    return VixenLibJNI.Mesh_SetIndex(swigCPtr, this, index, v);
  }

  public boolean AddIndex(long v) {
    return VixenLibJNI.Mesh_AddIndex(swigCPtr, this, v);
  }

  public long GetStartVtx() {
    return VixenLibJNI.Mesh_GetStartVtx(swigCPtr, this);
  }

  public boolean SetStartVtx(long index) {
    return VixenLibJNI.Mesh_SetStartVtx(swigCPtr, this, index);
  }

  public long GetEndVtx() {
    return VixenLibJNI.Mesh_GetEndVtx(swigCPtr, this);
  }

  public boolean SetEndVtx(long index) {
    return VixenLibJNI.Mesh_SetEndVtx(swigCPtr, this, index);
  }

  public long AddIndices(SWIGTYPE_p_int intArray, long size) {
    return VixenLibJNI.Mesh_AddIndices(swigCPtr, this, SWIGTYPE_p_int.getCPtr(intArray), size);
  }

  public long AddVertices(SWIGTYPE_p_float floatArray, long size) {
    return VixenLibJNI.Mesh_AddVertices(swigCPtr, this, SWIGTYPE_p_float.getCPtr(floatArray), size);
  }

  public Geometry Mul(Matrix arg0) {
    return new Geometry(VixenLibJNI.Mesh_Mul(swigCPtr, this, Matrix.getCPtr(arg0), arg0), false);
  }

  public boolean GetBound(Box3 OUTPUT) {
    return VixenLibJNI.Mesh_GetBound(swigCPtr, this, Box3.getCPtr(OUTPUT), OUTPUT);
  }

  public void Empty() {
    VixenLibJNI.Mesh_Empty(swigCPtr, this);
  }

  public boolean Copy(SharedObj arg0) {
    return VixenLibJNI.Mesh_Copy(swigCPtr, this, SharedObj.getCPtr(arg0), arg0);
  }

}
