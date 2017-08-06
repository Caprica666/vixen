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

public class Col4 : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal Col4(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(Col4 obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~Col4() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_Col4(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
    }
  }

		public static Col4 operator +(Col4 arg1, Col4 arg2)	{ return arg1.Add(arg2); }
		public static Col4 operator -(Col4 arg1, Col4 arg2)	{ return arg1.Sub(arg2); }
		public static Col4 operator *(Col4 vec, float val)	{ return vec.Mul(val); }
		public static Col4 operator /(Col4 vec, float val)	{ return vec.Div(val); }
	
  public float r {
    set {
      VixenLibPINVOKE.Col4_r_set(swigCPtr, value);
    } 
    get {
      float ret = VixenLibPINVOKE.Col4_r_get(swigCPtr);
      return ret;
    } 
  }

  public float g {
    set {
      VixenLibPINVOKE.Col4_g_set(swigCPtr, value);
    } 
    get {
      float ret = VixenLibPINVOKE.Col4_g_get(swigCPtr);
      return ret;
    } 
  }

  public float b {
    set {
      VixenLibPINVOKE.Col4_b_set(swigCPtr, value);
    } 
    get {
      float ret = VixenLibPINVOKE.Col4_b_get(swigCPtr);
      return ret;
    } 
  }

  public float a {
    set {
      VixenLibPINVOKE.Col4_a_set(swigCPtr, value);
    } 
    get {
      float ret = VixenLibPINVOKE.Col4_a_get(swigCPtr);
      return ret;
    } 
  }

  public Col4(float r, float g, float b, float a) : this(VixenLibPINVOKE.new_Col4__SWIG_0(r, g, b, a), true) {
  }

  public Col4(float r, float g, float b) : this(VixenLibPINVOKE.new_Col4__SWIG_1(r, g, b), true) {
  }

  public Col4(Col4 arg0) : this(VixenLibPINVOKE.new_Col4__SWIG_2(Col4.getCPtr(arg0)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public Col4() : this(VixenLibPINVOKE.new_Col4__SWIG_3(), true) {
  }

  public Col4(Color arg0) : this(VixenLibPINVOKE.new_Col4__SWIG_4(Color.getCPtr(arg0)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public Col4(uint arg0) : this(VixenLibPINVOKE.new_Col4__SWIG_5(arg0), true) {
  }

  public Col4(float[] floatArray) : this(VixenLibPINVOKE.new_Col4__SWIG_6(floatArray), true) {
  }

  public uint AsInt() {
    uint ret = VixenLibPINVOKE.Col4_AsInt(swigCPtr);
    return ret;
  }

  public Col4 Sub(Col4 arg0) {
    Col4 ret = new Col4(VixenLibPINVOKE.Col4_Sub(swigCPtr, Col4.getCPtr(arg0)), true);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public Col4 Add(Col4 arg0) {
    Col4 ret = new Col4(VixenLibPINVOKE.Col4_Add(swigCPtr, Col4.getCPtr(arg0)), true);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public Col4 Mul(float arg0) {
    Col4 ret = new Col4(VixenLibPINVOKE.Col4_Mul(swigCPtr, arg0), true);
    return ret;
  }

  public Col4 Div(float arg0) {
    Col4 ret = new Col4(VixenLibPINVOKE.Col4_Div(swigCPtr, arg0), true);
    return ret;
  }

  public bool Equals(Col4 arg0) {
    bool ret = VixenLibPINVOKE.Col4_Equals(swigCPtr, Col4.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Set(uint arg0) {
    VixenLibPINVOKE.Col4_Set__SWIG_0(swigCPtr, arg0);
  }

  public void Set(float R, float G, float B, float A) {
    VixenLibPINVOKE.Col4_Set__SWIG_1(swigCPtr, R, G, B, A);
  }

  public void Set(float R, float G, float B) {
    VixenLibPINVOKE.Col4_Set__SWIG_2(swigCPtr, R, G, B);
  }

}

}
