/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class MeshRef extends RefPtr {
  private long swigCPtr;

  public MeshRef(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.MeshRef_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(MeshRef obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_MeshRef(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public MeshRef(RefObj p) {
    this(VixenLibJNI.new_MeshRef__SWIG_0(RefObj.getCPtr(p), p), true);
  }

  public MeshRef() {
    this(VixenLibJNI.new_MeshRef__SWIG_1(), true);
  }

  public MeshRef(MeshRef src) {
    this(VixenLibJNI.new_MeshRef__SWIG_2(MeshRef.getCPtr(src), src), true);
  }

  public MeshRef(Mesh p) {
    this(VixenLibJNI.new_MeshRef__SWIG_3(Mesh.getCPtr(p), p), true);
  }

  public Mesh __ref__() 
	{
		jlong ptr = VixenLibJNI.MeshRef___ref__(swigCPtr, this);
		return (Mesh)  SharedObj.MakeObject(ptr, true);
	}


  public MeshRef Assign(MeshRef arg0) {
    return new MeshRef(VixenLibJNI.MeshRef_Assign__SWIG_0(swigCPtr, this, MeshRef.getCPtr(arg0), arg0), false);
  }

  public MeshRef Assign(Mesh arg0) {
    return new MeshRef(VixenLibJNI.MeshRef_Assign__SWIG_1(swigCPtr, this, Mesh.getCPtr(arg0), arg0), false);
  }

  public boolean Equal(MeshRef arg0) {
    return VixenLibJNI.MeshRef_Equal__SWIG_0(swigCPtr, this, MeshRef.getCPtr(arg0), arg0);
  }

  public boolean NotEqual(MeshRef arg0) {
    return VixenLibJNI.MeshRef_NotEqual__SWIG_0(swigCPtr, this, MeshRef.getCPtr(arg0), arg0);
  }

  public boolean Equal(Mesh arg0) {
    return VixenLibJNI.MeshRef_Equal__SWIG_1(swigCPtr, this, Mesh.getCPtr(arg0), arg0);
  }

  public boolean NotEqual(Mesh arg0) {
    return VixenLibJNI.MeshRef_NotEqual__SWIG_1(swigCPtr, this, Mesh.getCPtr(arg0), arg0);
  }

  public static MeshRef getNull() {
    long cPtr = VixenLibJNI.MeshRef_Null_get();
    return (cPtr == 0) ? null : new MeshRef(cPtr, false);
  }

}
