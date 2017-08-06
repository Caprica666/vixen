/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class SharedTree extends CoreTree {
  private long swigCPtr;

  public SharedTree(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.SharedTree_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(SharedTree obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_SharedTree(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public SharedTree() {
    this(VixenLibJNI.new_SharedTree(), true);
  }

  public void Empty() {
    VixenLibJNI.SharedTree_Empty(swigCPtr, this);
  }

  public boolean PutFirst(CoreTree child) {
    return VixenLibJNI.SharedTree_PutFirst(swigCPtr, this, CoreTree.getCPtr(child), child);
  }

  public boolean Append(CoreTree child) {
    return VixenLibJNI.SharedTree_Append(swigCPtr, this, CoreTree.getCPtr(child), child);
  }

  public boolean PutAfter(CoreTree after) {
    return VixenLibJNI.SharedTree_PutAfter(swigCPtr, this, CoreTree.getCPtr(after), after);
  }

  public boolean PutBefore(CoreTree before) {
    return VixenLibJNI.SharedTree_PutBefore(swigCPtr, this, CoreTree.getCPtr(before), before);
  }

  public boolean Remove(boolean free) {
    return VixenLibJNI.SharedTree_Remove__SWIG_0(swigCPtr, this, free);
  }

  public boolean Remove() {
    return VixenLibJNI.SharedTree_Remove__SWIG_1(swigCPtr, this);
  }

  public boolean Replace(CoreTree src) {
    return VixenLibJNI.SharedTree_Replace(swigCPtr, this, CoreTree.getCPtr(src), src);
  }

}
