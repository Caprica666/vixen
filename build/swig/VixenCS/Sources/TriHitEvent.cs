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

public class TriHitEvent : Event {
  private HandleRef swigCPtr;

  internal TriHitEvent(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.TriHitEvent_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(TriHitEvent obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~TriHitEvent() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_TriHitEvent(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public TriHitEvent(int code) : this(VixenLibPINVOKE.new_TriHitEvent__SWIG_0(code), true) {
  }

  public TriHitEvent() : this(VixenLibPINVOKE.new_TriHitEvent__SWIG_1(), true) {
  }

  public TriHitEvent(TriHitEvent src) : this(VixenLibPINVOKE.new_TriHitEvent__SWIG_2(TriHitEvent.getCPtr(src)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public Mesh Target {
		set
		{
			MeshRef oref = new MeshRef(value);
			VixenLibPINVOKE.TriHitEvent_Target_set(swigCPtr, MeshRef.getCPtr(oref));
		}
	
		get
		{
			IntPtr iref = VixenLibPINVOKE.TriHitEvent_Target_get(swigCPtr);
			if (iref == IntPtr.Zero)
				return null;
			MeshRef oref = new MeshRef(iref, false);
			return oref.__ref__();
		}
	
  }

  public Vec3 Intersect {
    set {
      VixenLibPINVOKE.TriHitEvent_Intersect_set(swigCPtr, Vec3.getCPtr(value));
      if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      Vec3 ret = new Vec3(VixenLibPINVOKE.TriHitEvent_Intersect_get(swigCPtr), false);
      if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public float Distance {
    set {
      VixenLibPINVOKE.TriHitEvent_Distance_set(swigCPtr, value);
    } 
    get {
      float ret = VixenLibPINVOKE.TriHitEvent_Distance_get(swigCPtr);
      return ret;
    } 
  }

  public long PrimIndex {
    set {
      VixenLibPINVOKE.TriHitEvent_PrimIndex_set(swigCPtr, value);
    } 
    get {
      long ret = VixenLibPINVOKE.TriHitEvent_PrimIndex_get(swigCPtr);
      return ret;
    } 
  }

  public long TriIndex {
    set {
      VixenLibPINVOKE.TriHitEvent_TriIndex_set(swigCPtr, value);
    } 
    get {
      long ret = VixenLibPINVOKE.TriHitEvent_TriIndex_get(swigCPtr);
      return ret;
    } 
  }

}

}
