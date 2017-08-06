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

public class ObjRef : RefPtr {
  private HandleRef swigCPtr;

  internal ObjRef(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.ObjRef_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(ObjRef obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~ObjRef() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_ObjRef(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public ObjRef(RefObj p) : this(VixenLibPINVOKE.new_ObjRef__SWIG_0(RefObj.getCPtr(p)), true) {
  }

  public ObjRef() : this(VixenLibPINVOKE.new_ObjRef__SWIG_1(), true) {
  }

  public ObjRef(ObjRef src) : this(VixenLibPINVOKE.new_ObjRef__SWIG_2(ObjRef.getCPtr(src)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public ObjRef(SharedObj p) : this(VixenLibPINVOKE.new_ObjRef__SWIG_3(SharedObj.getCPtr(p)), true) {
  }

  public new SharedObj __ref__() 
	{
		IntPtr cPtr = VixenLibPINVOKE.ObjRef___ref__(swigCPtr);
		return (SharedObj)  SharedObj.MakeObject(cPtr, true);
	}


  public ObjRef Assign(ObjRef arg0) {
    ObjRef ret = new ObjRef(VixenLibPINVOKE.ObjRef_Assign__SWIG_0(swigCPtr, ObjRef.getCPtr(arg0)), false);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public ObjRef Assign(SharedObj arg0) {
    ObjRef ret = new ObjRef(VixenLibPINVOKE.ObjRef_Assign__SWIG_1(swigCPtr, SharedObj.getCPtr(arg0)), false);
    return ret;
  }

  public bool Equal(ObjRef arg0) {
    bool ret = VixenLibPINVOKE.ObjRef_Equal__SWIG_0(swigCPtr, ObjRef.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool NotEqual(ObjRef arg0) {
    bool ret = VixenLibPINVOKE.ObjRef_NotEqual__SWIG_0(swigCPtr, ObjRef.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool Equal(SharedObj arg0) {
    bool ret = VixenLibPINVOKE.ObjRef_Equal__SWIG_1(swigCPtr, SharedObj.getCPtr(arg0));
    return ret;
  }

  public bool NotEqual(SharedObj arg0) {
    bool ret = VixenLibPINVOKE.ObjRef_NotEqual__SWIG_1(swigCPtr, SharedObj.getCPtr(arg0));
    return ret;
  }

  public static ObjRef Null {
    get {
      IntPtr cPtr = VixenLibPINVOKE.ObjRef_Null_get();
      ObjRef ret = (cPtr == IntPtr.Zero) ? null : new ObjRef(cPtr, false);
      return ret;
    } 
  }

}

}
