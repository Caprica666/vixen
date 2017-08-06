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

public class SpotLight : Light {
  private HandleRef swigCPtr;

  internal SpotLight(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.SpotLight_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(SpotLight obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~SpotLight() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_SpotLight(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public SpotLight() : this(VixenLibPINVOKE.new_SpotLight(), true) {
  }

  public float OuterAngle {
    set {
      VixenLibPINVOKE.SpotLight_OuterAngle_set(swigCPtr, value);
    } 
    get {
      float ret = VixenLibPINVOKE.SpotLight_OuterAngle_get(swigCPtr);
      return ret;
    } 
  }

  public float InnerAngle {
    set {
      VixenLibPINVOKE.SpotLight_InnerAngle_set(swigCPtr, value);
    } 
    get {
      float ret = VixenLibPINVOKE.SpotLight_InnerAngle_get(swigCPtr);
      return ret;
    } 
  }

  public int Decay {
    set {
      VixenLibPINVOKE.SpotLight_Decay_set(swigCPtr, value);
    } 
    get {
      int ret = VixenLibPINVOKE.SpotLight_Decay_get(swigCPtr);
      return ret;
    } 
  }

}

}
