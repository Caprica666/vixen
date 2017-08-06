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

public class SceneEvent : Event {
  private HandleRef swigCPtr;

  internal SceneEvent(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.SceneEvent_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(SceneEvent obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~SceneEvent() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_SceneEvent(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public SceneEvent(int code, Scene scene, SharedObj sender) : this(VixenLibPINVOKE.new_SceneEvent__SWIG_0(code, Scene.getCPtr(scene), SharedObj.getCPtr(sender)), true) {
  }

  public SceneEvent(int code, Scene scene) : this(VixenLibPINVOKE.new_SceneEvent__SWIG_1(code, Scene.getCPtr(scene)), true) {
  }

  public SceneEvent(int code) : this(VixenLibPINVOKE.new_SceneEvent__SWIG_2(code), true) {
  }

  public SceneEvent() : this(VixenLibPINVOKE.new_SceneEvent__SWIG_3(), true) {
  }

  public SceneEvent(Event src) : this(VixenLibPINVOKE.new_SceneEvent__SWIG_4(Event.getCPtr(src)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public Scene Target {
		set
		{
			SceneRef oref = new SceneRef(value);
			VixenLibPINVOKE.SceneEvent_Target_set(swigCPtr, SceneRef.getCPtr(oref));
		}
	
		get
		{
			IntPtr iref = VixenLibPINVOKE.SceneEvent_Target_get(swigCPtr);
			if (iref == IntPtr.Zero)
				return null;
			SceneRef oref = new SceneRef(iref, false);
			return oref.__ref__();
		}
	
  }

  public long WindowHandle {
    set {
      VixenLibPINVOKE.SceneEvent_WindowHandle_set(swigCPtr, value);
    } 
    get {
      long ret = VixenLibPINVOKE.SceneEvent_WindowHandle_get(swigCPtr);
      return ret;
    } 
  }

}

}
