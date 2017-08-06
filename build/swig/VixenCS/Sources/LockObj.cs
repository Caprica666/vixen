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

public class LockObj : RefObj {
  private HandleRef swigCPtr;

  internal LockObj(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.LockObj_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(LockObj obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~LockObj() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_LockObj(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public LockObj() : this(VixenLibPINVOKE.new_LockObj(), true) {
  }

  public bool MakeLock() {
    bool ret = VixenLibPINVOKE.LockObj_MakeLock(swigCPtr);
    return ret;
  }

  public void KillLock() {
    VixenLibPINVOKE.LockObj_KillLock(swigCPtr);
  }

  public bool Lock() {
    bool ret = VixenLibPINVOKE.LockObj_Lock(swigCPtr);
    return ret;
  }

  public bool Unlock() {
    bool ret = VixenLibPINVOKE.LockObj_Unlock(swigCPtr);
    return ret;
  }

  public bool TryLock() {
    bool ret = VixenLibPINVOKE.LockObj_TryLock(swigCPtr);
    return ret;
  }

}

}