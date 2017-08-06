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

public class CoreTree : SharedObj {
  private HandleRef swigCPtr;

  internal CoreTree(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.CoreTree_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(CoreTree obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~CoreTree() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_CoreTree(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public CoreTree() : this(VixenLibPINVOKE.new_CoreTree(), true) {
  }

  public CoreTree Parent() {
    IntPtr cPtr = VixenLibPINVOKE.CoreTree_Parent(swigCPtr);
    CoreTree ret = (cPtr == IntPtr.Zero) ? null : new CoreTree(cPtr, false);
    return ret;
  }

  public CoreTree First() {
    IntPtr cPtr = VixenLibPINVOKE.CoreTree_First(swigCPtr);
    CoreTree ret = (cPtr == IntPtr.Zero) ? null : new CoreTree(cPtr, false);
    return ret;
  }

  public CoreTree Last() {
    IntPtr cPtr = VixenLibPINVOKE.CoreTree_Last(swigCPtr);
    CoreTree ret = (cPtr == IntPtr.Zero) ? null : new CoreTree(cPtr, false);
    return ret;
  }

  public bool IsParent() {
    bool ret = VixenLibPINVOKE.CoreTree_IsParent(swigCPtr);
    return ret;
  }

  public bool IsChild() {
    bool ret = VixenLibPINVOKE.CoreTree_IsChild(swigCPtr);
    return ret;
  }

  public bool IsLast() {
    bool ret = VixenLibPINVOKE.CoreTree_IsLast(swigCPtr);
    return ret;
  }

  public int GetSize() {
    int ret = VixenLibPINVOKE.CoreTree_GetSize(swigCPtr);
    return ret;
  }

  public CoreTree GetAt(int n) {
    IntPtr cPtr = VixenLibPINVOKE.CoreTree_GetAt(swigCPtr, n);
    CoreTree ret = (cPtr == IntPtr.Zero) ? null : new CoreTree(cPtr, false);
    return ret;
  }

  public virtual bool PutFirst(CoreTree child) {
    bool ret = VixenLibPINVOKE.CoreTree_PutFirst(swigCPtr, CoreTree.getCPtr(child));
    return ret;
  }

  public virtual bool Append(CoreTree child) {
    bool ret = VixenLibPINVOKE.CoreTree_Append(swigCPtr, CoreTree.getCPtr(child));
    return ret;
  }

  public virtual bool PutAfter(CoreTree follow) {
    bool ret = VixenLibPINVOKE.CoreTree_PutAfter(swigCPtr, CoreTree.getCPtr(follow));
    return ret;
  }

  public virtual bool PutBefore(CoreTree before) {
    bool ret = VixenLibPINVOKE.CoreTree_PutBefore(swigCPtr, CoreTree.getCPtr(before));
    return ret;
  }

  public virtual bool Remove(bool free) {
    bool ret = VixenLibPINVOKE.CoreTree_Remove__SWIG_0(swigCPtr, free);
    return ret;
  }

  public virtual bool Remove() {
    bool ret = VixenLibPINVOKE.CoreTree_Remove__SWIG_1(swigCPtr);
    return ret;
  }

  public virtual void Empty() {
    VixenLibPINVOKE.CoreTree_Empty(swigCPtr);
  }

  public virtual bool Replace(CoreTree src) {
    bool ret = VixenLibPINVOKE.CoreTree_Replace(swigCPtr, CoreTree.getCPtr(src));
    return ret;
  }

  public static readonly int DEPTH_FIRST = VixenLibPINVOKE.CoreTree_DEPTH_FIRST_get();
  public static readonly int PARENTS_REVERSE = VixenLibPINVOKE.CoreTree_PARENTS_REVERSE_get();
  public static readonly int CHILDREN = VixenLibPINVOKE.CoreTree_CHILDREN_get();
  public static readonly int CHILDREN_REVERSE = VixenLibPINVOKE.CoreTree_CHILDREN_REVERSE_get();
  public static readonly int SEARCH_MASK = VixenLibPINVOKE.CoreTree_SEARCH_MASK_get();

}

}