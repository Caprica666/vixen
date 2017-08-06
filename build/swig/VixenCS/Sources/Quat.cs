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

public class Quat : Vec4 {
  private HandleRef swigCPtr;

  internal Quat(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.Quat_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(Quat obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~Quat() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_Quat(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

		public static Quat operator*(Quat vec1, Quat vec2)
		{
			Quat q = new Quat();
			q.Mul(vec1, vec2);
			return q;
		}
		
  public Quat() : this(VixenLibPINVOKE.new_Quat__SWIG_0(), true) {
  }

  public Quat(float X, float Y, float Z, float W) : this(VixenLibPINVOKE.new_Quat__SWIG_1(X, Y, Z, W), true) {
  }

  public Quat(Vec3 axis, float ang) : this(VixenLibPINVOKE.new_Quat__SWIG_2(Vec3.getCPtr(axis), ang), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public Quat(Matrix trans) : this(VixenLibPINVOKE.new_Quat__SWIG_3(Matrix.getCPtr(trans)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public Quat(Vec4 q) : this(VixenLibPINVOKE.new_Quat__SWIG_4(Vec4.getCPtr(q)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public Quat(Vec3 v1, Vec3 v2) : this(VixenLibPINVOKE.new_Quat__SWIG_5(Vec3.getCPtr(v1), Vec3.getCPtr(v2)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public new bool IsEmpty() {
    bool ret = VixenLibPINVOKE.Quat_IsEmpty(swigCPtr);
    return ret;
  }

  public void GetEulerAngles(Vec3 euler_angles) {
    VixenLibPINVOKE.Quat_GetEulerAngles(swigCPtr, Vec3.getCPtr(euler_angles));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void Set(Vec3 axis, float angle) {
    VixenLibPINVOKE.Quat_Set__SWIG_0(swigCPtr, Vec3.getCPtr(axis), angle);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void Set(Vec3 INPUT) {
    VixenLibPINVOKE.Quat_Set__SWIG_1(swigCPtr, Vec3.getCPtr(INPUT));
  }

  public void Set(Matrix mtx) {
    VixenLibPINVOKE.Quat_Set__SWIG_2(swigCPtr, Matrix.getCPtr(mtx));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void Set(Vec3 v1, Vec3 v2) {
    VixenLibPINVOKE.Quat_Set__SWIG_3(swigCPtr, Vec3.getCPtr(v1), Vec3.getCPtr(v2));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public new void Set(float x, float y, float z, float w) {
    VixenLibPINVOKE.Quat_Set__SWIG_4(swigCPtr, x, y, z, w);
  }

  public void Mul(Quat qL, Quat qR) {
    VixenLibPINVOKE.Quat_Mul(swigCPtr, Quat.getCPtr(qL), Quat.getCPtr(qR));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void Slerp(Quat p, Quat q, float t) {
    VixenLibPINVOKE.Quat_Slerp(swigCPtr, Quat.getCPtr(p), Quat.getCPtr(q), t);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void Conjugate() {
    VixenLibPINVOKE.Quat_Conjugate(swigCPtr);
  }

  public void Invert() {
    VixenLibPINVOKE.Quat_Invert(swigCPtr);
  }

  new public void Normalize() {
    VixenLibPINVOKE.Quat_Normalize(swigCPtr);
  }

  public void Squad(Quat p, Quat a, Quat b, Quat q, float t) {
    VixenLibPINVOKE.Quat_Squad(swigCPtr, Quat.getCPtr(p), Quat.getCPtr(a), Quat.getCPtr(b), Quat.getCPtr(q), t);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

}

}