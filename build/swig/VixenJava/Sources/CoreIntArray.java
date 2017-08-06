/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class CoreIntArray extends SharedObj {
  private long swigCPtr;

  public CoreIntArray(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.CoreIntArray_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(CoreIntArray obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_CoreIntArray(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public CoreIntArray(long size) {
    this(VixenLibJNI.new_CoreIntArray__SWIG_0(size), true);
  }

  public CoreIntArray() {
    this(VixenLibJNI.new_CoreIntArray__SWIG_1(), true);
  }

  public CoreIntArray(CoreIntArray arg0) {
    this(VixenLibJNI.new_CoreIntArray__SWIG_2(CoreIntArray.getCPtr(arg0), arg0), true);
  }

  public long GetSize() {
    return VixenLibJNI.CoreIntArray_GetSize(swigCPtr, this);
  }

  public long GetMaxSize() {
    return VixenLibJNI.CoreIntArray_GetMaxSize(swigCPtr, this);
  }

  public boolean SetSize(long size) {
    return VixenLibJNI.CoreIntArray_SetSize(swigCPtr, this, size);
  }

  public boolean SetMaxSize(long size) {
    return VixenLibJNI.CoreIntArray_SetMaxSize(swigCPtr, this, size);
  }

  public boolean SetAt(long index, int arg1) {
    return VixenLibJNI.CoreIntArray_SetAt(swigCPtr, this, index, arg1);
  }

  public SWIGTYPE_p_int GetAt(long index) {
    return new SWIGTYPE_p_int(VixenLibJNI.CoreIntArray_GetAt(swigCPtr, this, index), false);
  }

  public SWIGTYPE_p_int First() {
    long cPtr = VixenLibJNI.CoreIntArray_First__SWIG_0(swigCPtr, this);
    return (cPtr == 0) ? null : new SWIGTYPE_p_int(cPtr, false);
  }

  public SWIGTYPE_p_int Last() {
    long cPtr = VixenLibJNI.CoreIntArray_Last__SWIG_0(swigCPtr, this);
    return (cPtr == 0) ? null : new SWIGTYPE_p_int(cPtr, false);
  }

  public boolean IsEmpty() {
    return VixenLibJNI.CoreIntArray_IsEmpty(swigCPtr, this);
  }

  public void Empty() {
    VixenLibJNI.CoreIntArray_Empty(swigCPtr, this);
  }

  public void RemoveAt(long index, long size) {
    VixenLibJNI.CoreIntArray_RemoveAt__SWIG_0(swigCPtr, this, index, size);
  }

  public void RemoveAt(long index) {
    VixenLibJNI.CoreIntArray_RemoveAt__SWIG_1(swigCPtr, this, index);
  }

  public long Append(int arg0) {
    return VixenLibJNI.CoreIntArray_Append(swigCPtr, this, arg0);
  }

  public long Find(int arg0) {
    return VixenLibJNI.CoreIntArray_Find(swigCPtr, this, arg0);
  }

  public long Merge(CoreIntArray arg0) {
    return VixenLibJNI.CoreIntArray_Merge(swigCPtr, this, CoreIntArray.getCPtr(arg0), arg0);
  }

}
