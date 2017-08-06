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

public class Interpolator : Evaluator {
  private HandleRef swigCPtr;

  internal Interpolator(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.Interpolator_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(Interpolator obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~Interpolator() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_Interpolator(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public Interpolator() : this(VixenLibPINVOKE.new_Interpolator(), true) {
  }

  public virtual new int GetSize() {
    int ret = VixenLibPINVOKE.Interpolator_GetSize(swigCPtr);
    return ret;
  }

  public virtual void SetSize(int size) {
    VixenLibPINVOKE.Interpolator_SetSize(swigCPtr, size);
  }

  public virtual int GetMaxSize() {
    int ret = VixenLibPINVOKE.Interpolator_GetMaxSize(swigCPtr);
    return ret;
  }

  public virtual void SetMaxSize(int size) {
    VixenLibPINVOKE.Interpolator_SetMaxSize(swigCPtr, size);
  }

  public virtual float GetTime(int i) {
    float ret = VixenLibPINVOKE.Interpolator_GetTime(swigCPtr, i);
    return ret;
  }

  public virtual bool AddKey(float time, float[] floatArray) {
    bool ret = VixenLibPINVOKE.Interpolator_AddKey(swigCPtr, time, floatArray);
    return ret;
  }

  public virtual void ScaleKeys(float scale_factor) {
    VixenLibPINVOKE.Interpolator_ScaleKeys(swigCPtr, scale_factor);
  }

  public int InterpType {
    set {
      VixenLibPINVOKE.Interpolator_InterpType_set(swigCPtr, value);
    } 
    get {
      int ret = VixenLibPINVOKE.Interpolator_InterpType_get(swigCPtr);
      return ret;
    } 
  }

  public FloatArray Keys {
    set {
      VixenLibPINVOKE.Interpolator_Keys_set(swigCPtr, FloatArray.getCPtr(value));
    } 
	get
	{
		IntPtr cPtr = VixenLibPINVOKE.Interpolator_Keys_get(swigCPtr);
		return (FloatArray) SharedObj.MakeObject(cPtr, false);
	}

  }

}

}