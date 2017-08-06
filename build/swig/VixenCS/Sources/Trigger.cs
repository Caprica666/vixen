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

public class Trigger : Engine {
  private HandleRef swigCPtr;

  internal Trigger(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.Trigger_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(Trigger obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~Trigger() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_Trigger(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public Trigger() : this(VixenLibPINVOKE.new_Trigger__SWIG_0(), true) {
  }

  public Trigger(Trigger arg0) : this(VixenLibPINVOKE.new_Trigger__SWIG_1(Trigger.getCPtr(arg0)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool AddCollider(Model arg0) {
    bool ret = VixenLibPINVOKE.Trigger_AddCollider(swigCPtr, Model.getCPtr(arg0));
    return ret;
  }

  public bool RemoveCollider(Model arg0) {
    bool ret = VixenLibPINVOKE.Trigger_RemoveCollider(swigCPtr, Model.getCPtr(arg0));
    return ret;
  }

  public bool SetGeoSphere(Sphere arg0) {
    bool ret = VixenLibPINVOKE.Trigger_SetGeoSphere(swigCPtr, Sphere.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool SetGeoBox(Box3 arg0) {
    bool ret = VixenLibPINVOKE.Trigger_SetGeoBox(swigCPtr, Box3.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool GetGeoSphere(Sphere arg0) {
    bool ret = VixenLibPINVOKE.Trigger_GetGeoSphere(swigCPtr, Sphere.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool GetGeoBox(Box3 arg0) {
    bool ret = VixenLibPINVOKE.Trigger_GetGeoBox(swigCPtr, Box3.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int Options {
    set {
      VixenLibPINVOKE.Trigger_Options_set(swigCPtr, value);
    } 
    get {
      int ret = VixenLibPINVOKE.Trigger_Options_get(swigCPtr);
      return ret;
    } 
  }

  public Model Geometry {
    set {
      VixenLibPINVOKE.Trigger_Geometry_set(swigCPtr, Model.getCPtr(value));
    } 
	get
	{
		IntPtr cPtr = VixenLibPINVOKE.Trigger_Geometry_get(swigCPtr);
		return (Model) SharedObj.MakeObject(cPtr, false);
	}

  }

  public SharedObj Colliders {
    set {
      VixenLibPINVOKE.Trigger_Colliders_set(swigCPtr, SharedObj.getCPtr(value));
    } 
	get
	{
		IntPtr cPtr = VixenLibPINVOKE.Trigger_Colliders_get(swigCPtr);
		return (SharedObj) SharedObj.MakeObject(cPtr, false);
	}

  }

  public static readonly int SPHERE = VixenLibPINVOKE.Trigger_SPHERE_get();
  public static readonly int BOX = VixenLibPINVOKE.Trigger_BOX_get();
  public static readonly int FIT = VixenLibPINVOKE.Trigger_FIT_get();
  public static readonly int OUTSIDE = VixenLibPINVOKE.Trigger_OUTSIDE_get();
  public static readonly int INSIDE = VixenLibPINVOKE.Trigger_INSIDE_get();
  public static readonly int ENTER = VixenLibPINVOKE.Trigger_ENTER_get();
  public static readonly int LEAVE = VixenLibPINVOKE.Trigger_LEAVE_get();

}

}
