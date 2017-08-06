/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class SharedObjArray extends CoreObjArray {
  private long swigCPtr;

  public SharedObjArray(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.SharedObjArray_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(SharedObjArray obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_SharedObjArray(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public SharedObjArray(long size, ObjRef data) {
    this(VixenLibJNI.new_SharedObjArray__SWIG_0(size, ObjRef.getCPtr(data), data), true);
  }

  public SharedObjArray(long size) {
    this(VixenLibJNI.new_SharedObjArray__SWIG_1(size), true);
  }

  public SharedObjArray() {
    this(VixenLibJNI.new_SharedObjArray__SWIG_2(), true);
  }

  public boolean Copy(SharedObj arg0) {
    return VixenLibJNI.SharedObjArray_Copy(swigCPtr, this, SharedObj.getCPtr(arg0), arg0);
  }

  public int Save(SWIGTYPE_p_Vixen__Messenger arg0, int arg1) {
    return VixenLibJNI.SharedObjArray_Save(swigCPtr, this, SWIGTYPE_p_Vixen__Messenger.getCPtr(arg0), arg1);
  }

  public boolean SetAt(long index, ObjRef p) {
    return VixenLibJNI.SharedObjArray_SetAt(swigCPtr, this, index, ObjRef.getCPtr(p), p);
  }

  public void RemoveAt(long index, long size) {
    VixenLibJNI.SharedObjArray_RemoveAt__SWIG_0(swigCPtr, this, index, size);
  }

  public void RemoveAt(long index) {
    VixenLibJNI.SharedObjArray_RemoveAt__SWIG_1(swigCPtr, this, index);
  }

  public long Find(ObjRef e) {
    return VixenLibJNI.SharedObjArray_Find(swigCPtr, this, ObjRef.getCPtr(e), e);
  }

  public long Merge(SharedObjArray src) {
    return VixenLibJNI.SharedObjArray_Merge(swigCPtr, this, SharedObjArray.getCPtr(src), src);
  }

}
