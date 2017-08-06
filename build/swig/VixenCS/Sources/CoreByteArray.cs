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

public class CoreByteArray : SharedObj {
  private HandleRef swigCPtr;

  internal CoreByteArray(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.CoreByteArray_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(CoreByteArray obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~CoreByteArray() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_CoreByteArray(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public CoreByteArray(int size, int esize) : this(VixenLibPINVOKE.new_CoreByteArray__SWIG_0(size, esize), true) {
  }

  public CoreByteArray(int size) : this(VixenLibPINVOKE.new_CoreByteArray__SWIG_1(size), true) {
  }

  public CoreByteArray() : this(VixenLibPINVOKE.new_CoreByteArray__SWIG_2(), true) {
  }

  public int GetSize() {
    int ret = VixenLibPINVOKE.CoreByteArray_GetSize(swigCPtr);
    return ret;
  }

  public int GetMaxSize() {
    int ret = VixenLibPINVOKE.CoreByteArray_GetMaxSize(swigCPtr);
    return ret;
  }

  public bool SetSize(int arg0) {
    bool ret = VixenLibPINVOKE.CoreByteArray_SetSize(swigCPtr, arg0);
    return ret;
  }

  public bool SetMaxSize(int arg0) {
    bool ret = VixenLibPINVOKE.CoreByteArray_SetMaxSize(swigCPtr, arg0);
    return ret;
  }

  public int GetElemSize() {
    int ret = VixenLibPINVOKE.CoreByteArray_GetElemSize(swigCPtr);
    return ret;
  }

  public bool IsEmpty() {
    bool ret = VixenLibPINVOKE.CoreByteArray_IsEmpty(swigCPtr);
    return ret;
  }

  public void Empty() {
    VixenLibPINVOKE.CoreByteArray_Empty(swigCPtr);
  }

  public bool RemoveAt(int i, int n) {
    bool ret = VixenLibPINVOKE.CoreByteArray_RemoveAt__SWIG_0(swigCPtr, i, n);
    return ret;
  }

  public bool RemoveAt(int i) {
    bool ret = VixenLibPINVOKE.CoreByteArray_RemoveAt__SWIG_1(swigCPtr, i);
    return ret;
  }

  public int Merge(CoreByteArray arg0) {
    int ret = VixenLibPINVOKE.CoreByteArray_Merge(swigCPtr, CoreByteArray.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}

}
