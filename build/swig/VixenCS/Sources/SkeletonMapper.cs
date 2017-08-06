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

public class SkeletonMapper : PoseMapper {
  private HandleRef swigCPtr;

  internal SkeletonMapper(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.SkeletonMapper_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(SkeletonMapper obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~SkeletonMapper() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_SkeletonMapper(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public SkeletonMapper(SWIGTYPE_p_hkaSkeletonMapper mapper) : this(VixenLibPINVOKE.new_SkeletonMapper__SWIG_0(SWIGTYPE_p_hkaSkeletonMapper.getCPtr(mapper)), true) {
  }

  public SkeletonMapper() : this(VixenLibPINVOKE.new_SkeletonMapper__SWIG_1(), true) {
  }

  public RagDoll Source {
    set {
      VixenLibPINVOKE.SkeletonMapper_Source_set(swigCPtr, RagDoll.getCPtr(value));
    } 
	get
	{
		IntPtr cPtr = VixenLibPINVOKE.SkeletonMapper_Source_get(swigCPtr);
		return (RagDoll) SharedObj.MakeObject(cPtr, false);
	}

  }

}

}