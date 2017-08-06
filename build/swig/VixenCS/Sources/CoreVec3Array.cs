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

public class CoreVec3Array : SharedObj {
  private HandleRef swigCPtr;

  internal CoreVec3Array(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.CoreVec3Array_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(CoreVec3Array obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~CoreVec3Array() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_CoreVec3Array(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public CoreVec3Array(int nelems) : this(VixenLibPINVOKE.new_CoreVec3Array__SWIG_0(nelems), true) {
  }

  public CoreVec3Array() : this(VixenLibPINVOKE.new_CoreVec3Array__SWIG_1(), true) {
  }

  public CoreVec3Array(CoreVec3Array arg0) : this(VixenLibPINVOKE.new_CoreVec3Array__SWIG_2(CoreVec3Array.getCPtr(arg0)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public int GetSize() {
    int ret = VixenLibPINVOKE.CoreVec3Array_GetSize(swigCPtr);
    return ret;
  }

  public int GetMaxSize() {
    int ret = VixenLibPINVOKE.CoreVec3Array_GetMaxSize(swigCPtr);
    return ret;
  }

  public bool SetSize(int arg0) {
    bool ret = VixenLibPINVOKE.CoreVec3Array_SetSize(swigCPtr, arg0);
    return ret;
  }

  public bool SetMaxSize(int arg0) {
    bool ret = VixenLibPINVOKE.CoreVec3Array_SetMaxSize(swigCPtr, arg0);
    return ret;
  }

  public bool SetAt(int arg0, Vec3 arg1) {
    bool ret = VixenLibPINVOKE.CoreVec3Array_SetAt(swigCPtr, arg0, Vec3.getCPtr(arg1));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public Vec3 GetAt(int i) {
    Vec3 ret = new Vec3(VixenLibPINVOKE.CoreVec3Array_GetAt(swigCPtr, i), false);
    return ret;
  }

  public Vec3 First() {
    IntPtr cPtr = VixenLibPINVOKE.CoreVec3Array_First__SWIG_0(swigCPtr);
    Vec3 ret = (cPtr == IntPtr.Zero) ? null : new Vec3(cPtr, false);
    return ret;
  }

  public Vec3 Last() {
    IntPtr cPtr = VixenLibPINVOKE.CoreVec3Array_Last__SWIG_0(swigCPtr);
    Vec3 ret = (cPtr == IntPtr.Zero) ? null : new Vec3(cPtr, false);
    return ret;
  }

  public bool IsEmpty() {
    bool ret = VixenLibPINVOKE.CoreVec3Array_IsEmpty(swigCPtr);
    return ret;
  }

  public void Empty() {
    VixenLibPINVOKE.CoreVec3Array_Empty(swigCPtr);
  }

  public bool RemoveAt(int i, int n) {
    bool ret = VixenLibPINVOKE.CoreVec3Array_RemoveAt__SWIG_0(swigCPtr, i, n);
    return ret;
  }

  public bool RemoveAt(int i) {
    bool ret = VixenLibPINVOKE.CoreVec3Array_RemoveAt__SWIG_1(swigCPtr, i);
    return ret;
  }

  public virtual int Append(Vec3 arg0) {
    int ret = VixenLibPINVOKE.CoreVec3Array_Append(swigCPtr, Vec3.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual int Find(Vec3 arg0) {
    int ret = VixenLibPINVOKE.CoreVec3Array_Find(swigCPtr, Vec3.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual int Merge(CoreVec3Array arg0) {
    int ret = VixenLibPINVOKE.CoreVec3Array_Merge(swigCPtr, CoreVec3Array.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}

}