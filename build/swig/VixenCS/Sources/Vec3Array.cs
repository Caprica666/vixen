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

public class Vec3Array : SharedVec3Array {
  private HandleRef swigCPtr;

  internal Vec3Array(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.Vec3Array_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(Vec3Array obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~Vec3Array() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_Vec3Array(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public Vec3Array(int nelems) : this(VixenLibPINVOKE.new_Vec3Array__SWIG_0(nelems), true) {
  }

  public Vec3Array() : this(VixenLibPINVOKE.new_Vec3Array__SWIG_1(), true) {
  }

}

}
