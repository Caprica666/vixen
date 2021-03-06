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

public class ErrorEvent : Event {
  private HandleRef swigCPtr;

  internal ErrorEvent(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.ErrorEvent_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(ErrorEvent obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~ErrorEvent() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_ErrorEvent(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public ErrorEvent(SharedObj sender) : this(VixenLibPINVOKE.new_ErrorEvent__SWIG_0(SharedObj.getCPtr(sender)), true) {
  }

  public ErrorEvent() : this(VixenLibPINVOKE.new_ErrorEvent__SWIG_1(), true) {
  }

  public ErrorEvent(Event src) : this(VixenLibPINVOKE.new_ErrorEvent__SWIG_2(Event.getCPtr(src)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public int ErrCode {
    set {
      VixenLibPINVOKE.ErrorEvent_ErrCode_set(swigCPtr, value);
    } 
    get {
      int ret = VixenLibPINVOKE.ErrorEvent_ErrCode_get(swigCPtr);
      return ret;
    } 
  }

  public int ErrLevel {
    set {
      VixenLibPINVOKE.ErrorEvent_ErrLevel_set(swigCPtr, value);
    } 
    get {
      int ret = VixenLibPINVOKE.ErrorEvent_ErrLevel_get(swigCPtr);
      return ret;
    } 
  }

  public string ErrString {
    set {
      VixenLibPINVOKE.ErrorEvent_ErrString_set(swigCPtr, value);
      if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    } 
		get {
		  string ret = VixenLibPINVOKE.ErrorEvent_ErrString_get(swigCPtr);
      if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
		  return ret;
		}
	
  }

}

}
