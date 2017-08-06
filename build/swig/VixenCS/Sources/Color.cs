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

public class Color : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal Color(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(Color obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~Color() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_Color(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
    }
  }

		public static Color operator +(Color arg1, Color arg2)	{ return arg1.Add(arg2); }
		public static Color operator -(Color arg1, Color arg2)	{ return arg1.Sub(arg2); }
	
  public char b {
    set {
      VixenLibPINVOKE.Color_b_set(swigCPtr, value);
    } 
    get {
      char ret = VixenLibPINVOKE.Color_b_get(swigCPtr);
      return ret;
    } 
  }

  public char g {
    set {
      VixenLibPINVOKE.Color_g_set(swigCPtr, value);
    } 
    get {
      char ret = VixenLibPINVOKE.Color_g_get(swigCPtr);
      return ret;
    } 
  }

  public char r {
    set {
      VixenLibPINVOKE.Color_r_set(swigCPtr, value);
    } 
    get {
      char ret = VixenLibPINVOKE.Color_r_get(swigCPtr);
      return ret;
    } 
  }

  public char a {
    set {
      VixenLibPINVOKE.Color_a_set(swigCPtr, value);
    } 
    get {
      char ret = VixenLibPINVOKE.Color_a_get(swigCPtr);
      return ret;
    } 
  }

  public Color(int r, int g, int b, int a) : this(VixenLibPINVOKE.new_Color__SWIG_0(r, g, b, a), true) {
  }

  public Color(int r, int g, int b) : this(VixenLibPINVOKE.new_Color__SWIG_1(r, g, b), true) {
  }

  public Color(Color arg0) : this(VixenLibPINVOKE.new_Color__SWIG_2(Color.getCPtr(arg0)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public Color(Col4 arg0) : this(VixenLibPINVOKE.new_Color__SWIG_3(Col4.getCPtr(arg0)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public Color() : this(VixenLibPINVOKE.new_Color__SWIG_4(), true) {
  }

  public Color(uint arg0) : this(VixenLibPINVOKE.new_Color__SWIG_5(arg0), true) {
  }

  public uint AsInt() {
    uint ret = VixenLibPINVOKE.Color_AsInt(swigCPtr);
    return ret;
  }

  public Color Sub(Color arg0) {
    Color ret = new Color(VixenLibPINVOKE.Color_Sub(swigCPtr, Color.getCPtr(arg0)), true);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public Color Add(Color arg0) {
    Color ret = new Color(VixenLibPINVOKE.Color_Add(swigCPtr, Color.getCPtr(arg0)), true);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool Equals(Color arg0) {
    bool ret = VixenLibPINVOKE.Color_Equals(swigCPtr, Color.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Set(int R, int G, int B, int A) {
    VixenLibPINVOKE.Color_Set__SWIG_0(swigCPtr, R, G, B, A);
  }

  public void Set(int R, int G, int B) {
    VixenLibPINVOKE.Color_Set__SWIG_1(swigCPtr, R, G, B);
  }

}

}
