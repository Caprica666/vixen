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

public class SharedFloatArray : CoreFloatArray {
  private HandleRef swigCPtr;

  internal SharedFloatArray(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.SharedFloatArray_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(SharedFloatArray obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~SharedFloatArray() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_SharedFloatArray(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public SharedFloatArray(long size, SWIGTYPE_p_float data) : this(VixenLibPINVOKE.new_SharedFloatArray__SWIG_0(size, SWIGTYPE_p_float.getCPtr(data)), true) {
  }

  public SharedFloatArray(long size) : this(VixenLibPINVOKE.new_SharedFloatArray__SWIG_1(size), true) {
  }

  public SharedFloatArray() : this(VixenLibPINVOKE.new_SharedFloatArray__SWIG_2(), true) {
  }

  public new SWIGTYPE_p_float GetAt(long i) {
    SWIGTYPE_p_float ret = new SWIGTYPE_p_float(VixenLibPINVOKE.SharedFloatArray_GetAt(swigCPtr, i), false);
    return ret;
  }

  public new bool SetAt(long index, float p) {
    bool ret = VixenLibPINVOKE.SharedFloatArray_SetAt(swigCPtr, index, p);
    return ret;
  }

  public new void RemoveAt(long index, long size) {
    VixenLibPINVOKE.SharedFloatArray_RemoveAt__SWIG_0(swigCPtr, index, size);
  }

  public new void RemoveAt(long index) {
    VixenLibPINVOKE.SharedFloatArray_RemoveAt__SWIG_1(swigCPtr, index);
  }

  public virtual long Merge(SharedFloatArray src) {
    long ret = VixenLibPINVOKE.SharedFloatArray_Merge(swigCPtr, SharedFloatArray.getCPtr(src));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}

}
