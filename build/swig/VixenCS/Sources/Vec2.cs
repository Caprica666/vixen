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

public class Vec2 : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal Vec2(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(Vec2 obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~Vec2() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_Vec2(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
    }
  }

		public static Vec2 operator +(Vec2 arg1, Vec2 arg2)	{ return arg1.Add(arg2); }
		public static Vec2 operator -(Vec2 arg1, Vec2 arg2)	{ return arg1.Sub(arg2); }
		public static Vec2 operator *(Vec2 vec, float val)	{ return vec.Mul(val); }
		public static Vec2 operator /(Vec2 vec, float val)	{ return vec.Div(val); }
	
  public float x {
    set {
      VixenLibPINVOKE.Vec2_x_set(swigCPtr, value);
    } 
    get {
      float ret = VixenLibPINVOKE.Vec2_x_get(swigCPtr);
      return ret;
    } 
  }

  public float y {
    set {
      VixenLibPINVOKE.Vec2_y_set(swigCPtr, value);
    } 
    get {
      float ret = VixenLibPINVOKE.Vec2_y_get(swigCPtr);
      return ret;
    } 
  }

  public Vec2() : this(VixenLibPINVOKE.new_Vec2__SWIG_0(), true) {
  }

  public Vec2(float x, float y) : this(VixenLibPINVOKE.new_Vec2__SWIG_1(x, y), true) {
  }

  public Vec2(float[] floatArray) : this(VixenLibPINVOKE.new_Vec2__SWIG_2(floatArray), true) {
  }

  public Vec2(Vec2 arg0) : this(VixenLibPINVOKE.new_Vec2__SWIG_3(Vec2.getCPtr(arg0)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void Set(float x, float y) {
    VixenLibPINVOKE.Vec2_Set(swigCPtr, x, y);
  }

  public bool Equals(Vec2 arg0) {
    bool ret = VixenLibPINVOKE.Vec2_Equals(swigCPtr, Vec2.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public Vec2 Add(Vec2 arg0) {
    Vec2 ret = new Vec2(VixenLibPINVOKE.Vec2_Add(swigCPtr, Vec2.getCPtr(arg0)), true);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public Vec2 Sub(Vec2 arg0) {
    Vec2 ret = new Vec2(VixenLibPINVOKE.Vec2_Sub(swigCPtr, Vec2.getCPtr(arg0)), true);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public Vec2 Mul(float arg0) {
    Vec2 ret = new Vec2(VixenLibPINVOKE.Vec2_Mul(swigCPtr, arg0), true);
    return ret;
  }

  public Vec2 Div(float arg0) {
    Vec2 ret = new Vec2(VixenLibPINVOKE.Vec2_Div(swigCPtr, arg0), true);
    return ret;
  }

  public Vec2 Negate() {
    Vec2 ret = new Vec2(VixenLibPINVOKE.Vec2_Negate(swigCPtr), false);
    return ret;
  }

  public float Normalize() {
    float ret = VixenLibPINVOKE.Vec2_Normalize(swigCPtr);
    return ret;
  }

  public float Distance(Vec2 arg0) {
    float ret = VixenLibPINVOKE.Vec2_Distance(swigCPtr, Vec2.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public float DistanceSquared(Vec2 v) {
    float ret = VixenLibPINVOKE.Vec2_DistanceSquared(swigCPtr, Vec2.getCPtr(v));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public float Dot(Vec2 arg0) {
    float ret = VixenLibPINVOKE.Vec2_Dot__SWIG_0(swigCPtr, Vec2.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public float Dot(float[] floatArray) {
    float ret = VixenLibPINVOKE.Vec2_Dot__SWIG_1(swigCPtr, floatArray);
    return ret;
  }

  public float Length {
    set {
      VixenLibPINVOKE.Vec2_Length_set(swigCPtr, value);
    } 
    get {
      float ret = VixenLibPINVOKE.Vec2_Length_get(swigCPtr);
      return ret;
    } 
  }

  [System.ComponentModel.Browsable(false)]
  public float LengthSquared {
    set {
      VixenLibPINVOKE.Vec2_LengthSquared_set(swigCPtr, value);
    } 
    get {
      float ret = VixenLibPINVOKE.Vec2_LengthSquared_get(swigCPtr);
      return ret;
    } 
  }

}

}
