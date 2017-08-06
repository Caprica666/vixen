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

public class SharedObjArray : CoreObjArray {
  private HandleRef swigCPtr;

  internal SharedObjArray(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.SharedObjArray_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(SharedObjArray obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~SharedObjArray() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_SharedObjArray(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public SharedObjArray(long size, ObjRef data) : this(VixenLibPINVOKE.new_SharedObjArray__SWIG_0(size, ObjRef.getCPtr(data)), true) {
  }

  public SharedObjArray(long size) : this(VixenLibPINVOKE.new_SharedObjArray__SWIG_1(size), true) {
  }

  public SharedObjArray() : this(VixenLibPINVOKE.new_SharedObjArray__SWIG_2(), true) {
  }

  public new ObjRef GetAt(long i) {
    ObjRef ret = new ObjRef(VixenLibPINVOKE.SharedObjArray_GetAt(swigCPtr, i), false);
    return ret;
  }

  public new bool SetAt(long index, ObjRef p) {
    bool ret = VixenLibPINVOKE.SharedObjArray_SetAt(swigCPtr, index, ObjRef.getCPtr(p));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public new void RemoveAt(long index, long size) {
    VixenLibPINVOKE.SharedObjArray_RemoveAt__SWIG_0(swigCPtr, index, size);
  }

  public new void RemoveAt(long index) {
    VixenLibPINVOKE.SharedObjArray_RemoveAt__SWIG_1(swigCPtr, index);
  }

  public virtual long Merge(SharedObjArray src) {
    long ret = VixenLibPINVOKE.SharedObjArray_Merge(swigCPtr, SharedObjArray.getCPtr(src));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}

}
