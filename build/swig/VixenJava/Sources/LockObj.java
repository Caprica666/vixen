/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class LockObj extends RefObj {
  private long swigCPtr;

  public LockObj(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.LockObj_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(LockObj obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_LockObj(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public LockObj() {
    this(VixenLibJNI.new_LockObj(), true);
  }

  public boolean MakeLock() {
    return VixenLibJNI.LockObj_MakeLock(swigCPtr, this);
  }

  public void KillLock() {
    VixenLibJNI.LockObj_KillLock(swigCPtr, this);
  }

  public boolean Lock() {
    return VixenLibJNI.LockObj_Lock(swigCPtr, this);
  }

  public boolean Unlock() {
    return VixenLibJNI.LockObj_Unlock(swigCPtr, this);
  }

  public boolean TryLock() {
    return VixenLibJNI.LockObj_TryLock(swigCPtr, this);
  }

}
