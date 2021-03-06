/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class Pose extends SharedObj {
  private long swigCPtr;

  public Pose(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.Pose_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(Pose obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_Pose(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public Pose(Skeleton skel, int space) {
    this(VixenLibJNI.new_Pose__SWIG_0(Skeleton.getCPtr(skel), skel, space), true);
  }

  public Pose(Skeleton skel) {
    this(VixenLibJNI.new_Pose__SWIG_1(Skeleton.getCPtr(skel), skel), true);
  }

  public Pose() {
    this(VixenLibJNI.new_Pose__SWIG_2(), true);
  }

  public int GetNumBones() {
    return VixenLibJNI.Pose_GetNumBones(swigCPtr, this);
  }

  public int GetCoordSpace() {
    return VixenLibJNI.Pose_GetCoordSpace(swigCPtr, this);
  }

  public Skeleton GetSkeleton() 
	{
		jlong ptr = VixenLibJNI.Pose_GetSkeleton(swigCPtr, this);
		return (Skeleton)  SharedObj.MakeObject(ptr, true);
	}


  public void ClearRotations() {
    VixenLibJNI.Pose_ClearRotations(swigCPtr, this);
  }

  public Vec3 GetWorldPosition(int i) {
    return new Vec3(VixenLibJNI.Pose_GetWorldPosition(swigCPtr, this, i), false);
  }

  public void GetWorldPositions(Vec3 positions) {
    VixenLibJNI.Pose_GetWorldPositions__SWIG_0(swigCPtr, this, Vec3.getCPtr(positions), positions);
  }

  public void GetWorldMatrix(int i, Matrix mtx) {
    VixenLibJNI.Pose_GetWorldMatrix(swigCPtr, this, i, Matrix.getCPtr(mtx), mtx);
  }

  public void SetWorldMatrix(int i, Matrix mtx) {
    VixenLibJNI.Pose_SetWorldMatrix(swigCPtr, this, i, Matrix.getCPtr(mtx), mtx);
  }

  public void GetWorldRotations(Quat rotations) {
    VixenLibJNI.Pose_GetWorldRotations__SWIG_0(swigCPtr, this, Quat.getCPtr(rotations), rotations);
  }

  public Quat GetWorldRotation(int i) {
    return new Quat(VixenLibJNI.Pose_GetWorldRotation(swigCPtr, this, i), false);
  }

  public void SetWorldRotation(int i, Quat rot) {
    VixenLibJNI.Pose_SetWorldRotation(swigCPtr, this, i, Quat.getCPtr(rot), rot);
  }

  public void GetLocalMatrix(int i, Matrix mtx) {
    VixenLibJNI.Pose_GetLocalMatrix(swigCPtr, this, i, Matrix.getCPtr(mtx), mtx);
  }

  public Quat GetLocalRotation(int i) {
    return new Quat(VixenLibJNI.Pose_GetLocalRotation(swigCPtr, this, i), false);
  }

  public void SetLocalRotation(int i, Quat rot) {
    VixenLibJNI.Pose_SetLocalRotation(swigCPtr, this, i, Quat.getCPtr(rot), rot);
  }

  public Vec3 GetLocalPosition(int i) {
    return new Vec3(VixenLibJNI.Pose_GetLocalPosition(swigCPtr, this, i), false);
  }

  public boolean Compare(Pose src) {
    return VixenLibJNI.Pose_Compare(swigCPtr, this, Pose.getCPtr(src), src);
  }

  public void SetPosition(Vec3 arg0) {
    VixenLibJNI.Pose_SetPosition(swigCPtr, this, Vec3.getCPtr(arg0), arg0);
  }

  public boolean Sync() {
    return VixenLibJNI.Pose_Sync(swigCPtr, this);
  }

  public boolean Copy(SharedObj arg0) {
    return VixenLibJNI.Pose_Copy(swigCPtr, this, SharedObj.getCPtr(arg0), arg0);
  }

  public void SetWorldPositions(SWIGTYPE_p_float floatArray) {
    VixenLibJNI.Pose_SetWorldPositions(swigCPtr, this, SWIGTYPE_p_float.getCPtr(floatArray));
  }

  public void GetWorldPositions(SWIGTYPE_p_float floatArray) {
    VixenLibJNI.Pose_GetWorldPositions__SWIG_1(swigCPtr, this, SWIGTYPE_p_float.getCPtr(floatArray));
  }

  public void SetWorldRotations(SWIGTYPE_p_float floatArray) {
    VixenLibJNI.Pose_SetWorldRotations(swigCPtr, this, SWIGTYPE_p_float.getCPtr(floatArray));
  }

  public void GetWorldRotations(SWIGTYPE_p_float floatArray) {
    VixenLibJNI.Pose_GetWorldRotations__SWIG_1(swigCPtr, this, SWIGTYPE_p_float.getCPtr(floatArray));
  }

  public void SetLocalRotations(SWIGTYPE_p_float floatArray) {
    VixenLibJNI.Pose_SetLocalRotations(swigCPtr, this, SWIGTYPE_p_float.getCPtr(floatArray));
  }

  public final static int WORLD = VixenLibJNI.Pose_WORLD_get();
  public final static int BIND_POSE_RELATIVE = VixenLibJNI.Pose_BIND_POSE_RELATIVE_get();
  public final static int SKELETON = VixenLibJNI.Pose_SKELETON_get();
  public final static int BIND_POSE = VixenLibJNI.Pose_BIND_POSE_get();

}
