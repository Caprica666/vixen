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

public class PhongMaterial : Material {
  private HandleRef swigCPtr;

  internal PhongMaterial(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.PhongMaterial_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(PhongMaterial obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~PhongMaterial() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_PhongMaterial(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public PhongMaterial() : this(VixenLibPINVOKE.new_PhongMaterial__SWIG_0(), true) {
  }

  public PhongMaterial(Col4 c) : this(VixenLibPINVOKE.new_PhongMaterial__SWIG_1(Col4.getCPtr(c)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public PhongMaterial(PhongMaterial src) : this(VixenLibPINVOKE.new_PhongMaterial__SWIG_2(PhongMaterial.getCPtr(src)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool Equal(PhongMaterial arg0) {
    bool ret = VixenLibPINVOKE.PhongMaterial_Equal(swigCPtr, PhongMaterial.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool NotEqual(PhongMaterial src) {
    bool ret = VixenLibPINVOKE.PhongMaterial_NotEqual(swigCPtr, PhongMaterial.getCPtr(src));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public Col4 Emission {
    set {
      VixenLibPINVOKE.PhongMaterial_Emission_set(swigCPtr, Col4.getCPtr(value));
      if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      Col4 ret = new Col4(VixenLibPINVOKE.PhongMaterial_Emission_get(swigCPtr), false);
      if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public float Shine {
    set {
      VixenLibPINVOKE.PhongMaterial_Shine_set(swigCPtr, value);
    } 
    get {
      float ret = VixenLibPINVOKE.PhongMaterial_Shine_get(swigCPtr);
      return ret;
    } 
  }

  public Col4 Ambient {
    set {
      VixenLibPINVOKE.PhongMaterial_Ambient_set(swigCPtr, Col4.getCPtr(value));
      if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      Col4 ret = new Col4(VixenLibPINVOKE.PhongMaterial_Ambient_get(swigCPtr), false);
      if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public Col4 Diffuse {
    set {
      VixenLibPINVOKE.PhongMaterial_Diffuse_set(swigCPtr, Col4.getCPtr(value));
      if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      Col4 ret = new Col4(VixenLibPINVOKE.PhongMaterial_Diffuse_get(swigCPtr), false);
      if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public Col4 Specular {
    set {
      VixenLibPINVOKE.PhongMaterial_Specular_set(swigCPtr, Col4.getCPtr(value));
      if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      Col4 ret = new Col4(VixenLibPINVOKE.PhongMaterial_Specular_get(swigCPtr), false);
      if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

}

}
