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

public class Sphere : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal Sphere(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(Sphere obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~Sphere() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_Sphere(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
    }
  }

  public Sphere() : this(VixenLibPINVOKE.new_Sphere__SWIG_0(), true) {
  }

  public Sphere(Box3 arg0) : this(VixenLibPINVOKE.new_Sphere__SWIG_1(Box3.getCPtr(arg0)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public Sphere(Sphere arg0) : this(VixenLibPINVOKE.new_Sphere__SWIG_2(Sphere.getCPtr(arg0)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public Sphere(Vec3 center, float radius) : this(VixenLibPINVOKE.new_Sphere__SWIG_3(Vec3.getCPtr(center), radius), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void Set(Vec3 center, float radius) {
    VixenLibPINVOKE.Sphere_Set(swigCPtr, Vec3.getCPtr(center), radius);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void Empty() {
    VixenLibPINVOKE.Sphere_Empty(swigCPtr);
  }

  public bool IsEmpty() {
    bool ret = VixenLibPINVOKE.Sphere_IsEmpty(swigCPtr);
    return ret;
  }

  public void Extend(Sphere arg0) {
    VixenLibPINVOKE.Sphere_Extend__SWIG_0(swigCPtr, Sphere.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void Extend(Box3 arg0) {
    VixenLibPINVOKE.Sphere_Extend__SWIG_1(swigCPtr, Box3.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public Sphere Mul(Matrix arg0) {
    Sphere ret = new Sphere(VixenLibPINVOKE.Sphere_Mul(swigCPtr, Matrix.getCPtr(arg0)), false);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public Sphere Assign(Sphere arg0) {
    Sphere ret = new Sphere(VixenLibPINVOKE.Sphere_Assign__SWIG_0(swigCPtr, Sphere.getCPtr(arg0)), false);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public Sphere Assign(Box3 arg0) {
    Sphere ret = new Sphere(VixenLibPINVOKE.Sphere_Assign__SWIG_1(swigCPtr, Box3.getCPtr(arg0)), false);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool Equal(Sphere arg0) {
    bool ret = VixenLibPINVOKE.Sphere_Equal(swigCPtr, Sphere.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool NotEqual(Sphere arg0) {
    bool ret = VixenLibPINVOKE.Sphere_NotEqual(swigCPtr, Sphere.getCPtr(arg0));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public Vec3 Center {
    set {
      VixenLibPINVOKE.Sphere_Center_set(swigCPtr, Vec3.getCPtr(value));
      if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      Vec3 ret = new Vec3(VixenLibPINVOKE.Sphere_Center_get(swigCPtr), false);
      if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public float Radius {
    set {
      VixenLibPINVOKE.Sphere_Radius_set(swigCPtr, value);
    } 
    get {
      float ret = VixenLibPINVOKE.Sphere_Radius_get(swigCPtr);
      return ret;
    } 
  }

}

}
