/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace Vixen {

using System;
using System.Runtime.InteropServices;

public class VertexPool : SharedObj {
  private HandleRef swigCPtr;

  internal VertexPool(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.VertexPool_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(VertexPool obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~VertexPool() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_VertexPool(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public VertexPool(int style) : this(VixenLibPINVOKE.new_VertexPool__SWIG_0(style), true) {
  }

  public VertexPool() : this(VixenLibPINVOKE.new_VertexPool__SWIG_1(), true) {
  }

  public VertexPool(string layout_desc) : this(VixenLibPINVOKE.new_VertexPool__SWIG_2(layout_desc), true) {
  }

  public int GetStyle() {
    int ret = VixenLibPINVOKE.VertexPool_GetStyle(swigCPtr);
    return ret;
  }

  public void SetStyle(int arg0) {
    VixenLibPINVOKE.VertexPool_SetStyle(swigCPtr, arg0);
  }

  public int GetVtxSize() {
    int ret = VixenLibPINVOKE.VertexPool_GetVtxSize(swigCPtr);
    return ret;
  }

  public long GetNumVtx() {
    long ret = VixenLibPINVOKE.VertexPool_GetNumVtx(swigCPtr);
    return ret;
  }

  public long GetMaxVtx() {
    long ret = VixenLibPINVOKE.VertexPool_GetMaxVtx(swigCPtr);
    return ret;
  }

  public virtual bool SetNumVtx(long size) {
    bool ret = VixenLibPINVOKE.VertexPool_SetNumVtx(swigCPtr, size);
    return ret;
  }

  public virtual bool SetMaxVtx(long size) {
    bool ret = VixenLibPINVOKE.VertexPool_SetMaxVtx(swigCPtr, size);
    return ret;
  }

  public virtual bool Grow(long size) {
    bool ret = VixenLibPINVOKE.VertexPool_Grow(swigCPtr, size);
    return ret;
  }

  public virtual bool GetBound(Box3 arg0) {
    bool ret = VixenLibPINVOKE.VertexPool_GetBound(swigCPtr, Box3.getCPtr(arg0));
    return ret;
  }

  public virtual long Find(float[] floatArray) {
    long ret = VixenLibPINVOKE.VertexPool_Find(swigCPtr, floatArray);
    return ret;
  }

  public virtual long AddVertices(float[] floatArray, long n) {
    long ret = VixenLibPINVOKE.VertexPool_AddVertices(swigCPtr, floatArray, n);
    return ret;
  }

  public virtual SWIGTYPE_p_float PadVertices(float[] floatArray, long size, long srcstride) {
    IntPtr cPtr = VixenLibPINVOKE.VertexPool_PadVertices(swigCPtr, floatArray, size, srcstride);
    SWIGTYPE_p_float ret = (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_float(cPtr, false);
    return ret;
  }

  public virtual VertexPool Mul(Matrix arg0) {
    VertexPool ret = new VertexPool(VixenLibPINVOKE.VertexPool_Mul(swigCPtr, Matrix.getCPtr(arg0)), false);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static readonly int MORPH = VixenLibPINVOKE.VertexPool_MORPH_get();
  public static readonly int LOCATIONS = VixenLibPINVOKE.VertexPool_LOCATIONS_get();
  public static readonly int COLORS = VixenLibPINVOKE.VertexPool_COLORS_get();
  public static readonly int NORMALS = VixenLibPINVOKE.VertexPool_NORMALS_get();
  public static readonly int TEXCOORDS = VixenLibPINVOKE.VertexPool_TEXCOORDS_get();
  public static readonly int TANGENTS = VixenLibPINVOKE.VertexPool_TANGENTS_get();
  public static readonly int INTEGER = VixenLibPINVOKE.VertexPool_INTEGER_get();
  public static readonly int MAX_TEXCOORDS = VixenLibPINVOKE.VertexPool_MAX_TEXCOORDS_get();
  public static readonly int MAX_SLOTS = VixenLibPINVOKE.VertexPool_MAX_SLOTS_get();
  public static readonly int MAX_VTX_SIZE = VixenLibPINVOKE.VertexPool_MAX_VTX_SIZE_get();

}

}
