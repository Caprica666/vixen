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

public class KeyColor : KeyFramer {
  private HandleRef swigCPtr;

  internal KeyColor(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.KeyColor_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(KeyColor obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~KeyColor() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_KeyColor(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public KeyColor() : this(VixenLibPINVOKE.new_KeyColor(), true) {
  }

  public Col4 GetVal(int i) {
    Col4 ret = new Col4(VixenLibPINVOKE.KeyColor_GetVal(swigCPtr, i), false);
    return ret;
  }

  public bool AddVal(float t, Col4 key) {
    bool ret = VixenLibPINVOKE.KeyColor_AddVal(swigCPtr, t, Col4.getCPtr(key));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}

}
