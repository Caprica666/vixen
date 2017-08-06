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

public class Scriptor : Sequencer {
  private HandleRef swigCPtr;

  internal Scriptor(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.Scriptor_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(Scriptor obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~Scriptor() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_Scriptor(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public Scriptor() : this(VixenLibPINVOKE.new_Scriptor(), true) {
  }

  public int GetError() {
    int ret = VixenLibPINVOKE.Scriptor_GetError(swigCPtr);
    return ret;
  }

  public static SharedObj Find(string name) 
	{
		IntPtr cPtr = VixenLibPINVOKE.Scriptor_Find(name);
		return (SharedObj)  SharedObj.MakeObject(cPtr, true);
	}


  public virtual void Exec(string scriptdata) {
    VixenLibPINVOKE.Scriptor_Exec__SWIG_0(swigCPtr, scriptdata);
  }

  public virtual void Exec() {
    VixenLibPINVOKE.Scriptor_Exec__SWIG_1(swigCPtr);
  }

  public virtual bool WhenEvent(int code, SharedObj sender, string s) {
    bool ret = VixenLibPINVOKE.Scriptor_WhenEvent(swigCPtr, code, SharedObj.getCPtr(sender), s);
    return ret;
  }

  public virtual void LoadScript(string file) {
    VixenLibPINVOKE.Scriptor_LoadScript(swigCPtr, file);
  }

  public static void SetAnimationScale(float arg0) {
    VixenLibPINVOKE.Scriptor_SetAnimationScale(arg0);
  }

  public static float GetAnimationScale() {
    float ret = VixenLibPINVOKE.Scriptor_GetAnimationScale();
    return ret;
  }

  public string Directory {
    set {
      VixenLibPINVOKE.Scriptor_Directory_set(swigCPtr, value);
    } 
    get {
      string ret = VixenLibPINVOKE.Scriptor_Directory_get(swigCPtr);
      return ret;
    } 
  }

}

}
