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

public class KinectTracker : BodyTracker {
  private HandleRef swigCPtr;

  internal KinectTracker(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.KinectTracker_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(KinectTracker obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~KinectTracker() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_KinectTracker(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public KinectTracker(string configfile) : this(VixenLibPINVOKE.new_KinectTracker__SWIG_0(configfile), true) {
  }

  public KinectTracker() : this(VixenLibPINVOKE.new_KinectTracker__SWIG_1(), true) {
  }

  public KinectTracker(KinectTracker src) : this(VixenLibPINVOKE.new_KinectTracker__SWIG_2(KinectTracker.getCPtr(src)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public static readonly int HEAD = VixenLibPINVOKE.KinectTracker_HEAD_get();
  public static readonly int NECK = VixenLibPINVOKE.KinectTracker_NECK_get();
  public static readonly int TORSO = VixenLibPINVOKE.KinectTracker_TORSO_get();
  public static readonly int WAIST = VixenLibPINVOKE.KinectTracker_WAIST_get();
  public static readonly int RIGHT_COLLAR = VixenLibPINVOKE.KinectTracker_RIGHT_COLLAR_get();
  public static readonly int RIGHT_SHOULDER = VixenLibPINVOKE.KinectTracker_RIGHT_SHOULDER_get();
  public static readonly int RIGHT_ELBOW = VixenLibPINVOKE.KinectTracker_RIGHT_ELBOW_get();
  public static readonly int RIGHT_WRIST = VixenLibPINVOKE.KinectTracker_RIGHT_WRIST_get();
  public static readonly int RIGHT_HAND = VixenLibPINVOKE.KinectTracker_RIGHT_HAND_get();
  public static readonly int RIGHT_FINGER = VixenLibPINVOKE.KinectTracker_RIGHT_FINGER_get();
  public static readonly int LEFT_COLLAR = VixenLibPINVOKE.KinectTracker_LEFT_COLLAR_get();
  public static readonly int LEFT_SHOULDER = VixenLibPINVOKE.KinectTracker_LEFT_SHOULDER_get();
  public static readonly int LEFT_ELBOW = VixenLibPINVOKE.KinectTracker_LEFT_ELBOW_get();
  public static readonly int LEFT_WRIST = VixenLibPINVOKE.KinectTracker_LEFT_WRIST_get();
  public static readonly int LEFT_HAND = VixenLibPINVOKE.KinectTracker_LEFT_HAND_get();
  public static readonly int LEFT_FINGER = VixenLibPINVOKE.KinectTracker_LEFT_FINGER_get();
  public static readonly int RIGHT_HIP = VixenLibPINVOKE.KinectTracker_RIGHT_HIP_get();
  public static readonly int RIGHT_KNEE = VixenLibPINVOKE.KinectTracker_RIGHT_KNEE_get();
  public static readonly int RIGHT_ANKLE = VixenLibPINVOKE.KinectTracker_RIGHT_ANKLE_get();
  public static readonly int RIGHT_FOOT = VixenLibPINVOKE.KinectTracker_RIGHT_FOOT_get();
  public static readonly int LEFT_HIP = VixenLibPINVOKE.KinectTracker_LEFT_HIP_get();
  public static readonly int LEFT_KNEE = VixenLibPINVOKE.KinectTracker_LEFT_KNEE_get();
  public static readonly int LEFT_ANKLE = VixenLibPINVOKE.KinectTracker_LEFT_ANKLE_get();
  public static readonly int LEFT_FOOT = VixenLibPINVOKE.KinectTracker_LEFT_FOOT_get();
  public static readonly int NUM_BONES = VixenLibPINVOKE.KinectTracker_NUM_BONES_get();

}

}