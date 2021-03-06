/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class Model extends Group {
  private long swigCPtr;

  public Model(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.Model_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(Model obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_Model(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public Model Parent() 
	{
		jlong ptr = VixenLibJNI.Model_Parent(swigCPtr, this);
		return (Model)  SharedObj.MakeObject(ptr, true);
	}


  public Model First() 
	{
		jlong ptr = VixenLibJNI.Model_First(swigCPtr, this);
		return (Model)  SharedObj.MakeObject(ptr, true);
	}


  public Model Last() 
	{
		jlong ptr = VixenLibJNI.Model_Last(swigCPtr, this);
		return (Model)  SharedObj.MakeObject(ptr, true);
	}


  public Model Next() 
	{
		jlong ptr = VixenLibJNI.Model_Next(swigCPtr, this);
		return (Model)  SharedObj.MakeObject(ptr, true);
	}


  public Model GetAt(int i) 
	{
		jlong ptr = VixenLibJNI.Model_GetAt(swigCPtr, this, i);
		return (Model)  SharedObj.MakeObject(ptr, true);
	}


  public Model(Model arg0) {
    this(VixenLibJNI.new_Model__SWIG_0(Model.getCPtr(arg0), arg0), true);
  }

  public Model() {
    this(VixenLibJNI.new_Model__SWIG_1(), true);
  }

  public int GetHints() {
    return VixenLibJNI.Model_GetHints(swigCPtr, this);
  }

  public void SetHints(int hints, boolean descend) {
    VixenLibJNI.Model_SetHints__SWIG_0(swigCPtr, this, hints, descend);
  }

  public void SetHints(int hints) {
    VixenLibJNI.Model_SetHints__SWIG_1(swigCPtr, this, hints);
  }

  public Vec3 GetCenter(int transformtype) {
    return new Vec3(VixenLibJNI.Model_GetCenter__SWIG_0(swigCPtr, this, transformtype), true);
  }

  public Vec3 GetCenter() {
    return new Vec3(VixenLibJNI.Model_GetCenter__SWIG_1(swigCPtr, this), true);
  }

  public boolean GetCenter(Vec3 OUTPUT, int transformtype) {
    return VixenLibJNI.Model_GetCenter__SWIG_2(swigCPtr, this, Vec3.getCPtr(OUTPUT), OUTPUT, transformtype);
  }

  public boolean GetCenter(Vec3 OUTPUT) {
    return VixenLibJNI.Model_GetCenter__SWIG_3(swigCPtr, this, Vec3.getCPtr(OUTPUT), OUTPUT);
  }

  public Vec3 GetDirection() {
    return new Vec3(VixenLibJNI.Model_GetDirection(swigCPtr, this), true);
  }

  public boolean GetBound(Sphere OUTPUT, int transformtype) {
    return VixenLibJNI.Model_GetBound__SWIG_0(swigCPtr, this, Sphere.getCPtr(OUTPUT), OUTPUT, transformtype);
  }

  public boolean GetBound(Sphere OUTPUT) {
    return VixenLibJNI.Model_GetBound__SWIG_1(swigCPtr, this, Sphere.getCPtr(OUTPUT), OUTPUT);
  }

  public boolean GetBound(Box3 OUTPUT, int transformtype) {
    return VixenLibJNI.Model_GetBound__SWIG_2(swigCPtr, this, Box3.getCPtr(OUTPUT), OUTPUT, transformtype);
  }

  public boolean GetBound(Box3 OUTPUT) {
    return VixenLibJNI.Model_GetBound__SWIG_3(swigCPtr, this, Box3.getCPtr(OUTPUT), OUTPUT);
  }

  public void SetBound(Box3 INPUT) {
    VixenLibJNI.Model_SetBound__SWIG_0(swigCPtr, this, Box3.getCPtr(INPUT), INPUT);
  }

  public void SetBound(Sphere INPUT) {
    VixenLibJNI.Model_SetBound__SWIG_1(swigCPtr, this, Sphere.getCPtr(INPUT), INPUT);
  }

  public boolean IsCulling() {
    return VixenLibJNI.Model_IsCulling(swigCPtr, this);
  }

  public void SetCulling(boolean cull, boolean descend) {
    VixenLibJNI.Model_SetCulling__SWIG_0(swigCPtr, this, cull, descend);
  }

  public void SetCulling(boolean cull) {
    VixenLibJNI.Model_SetCulling__SWIG_1(swigCPtr, this, cull);
  }

  public boolean CalcSphere(Sphere arg0) {
    return VixenLibJNI.Model_CalcSphere(swigCPtr, this, Sphere.getCPtr(arg0), arg0);
  }

  public boolean CalcBound(Box3 arg0) {
    return VixenLibJNI.Model_CalcBound(swigCPtr, this, Box3.getCPtr(arg0), arg0);
  }

  public void Reset() {
    VixenLibJNI.Model_Reset(swigCPtr, this);
  }

  public void SetTranslation(Vec3 INPUT) {
    VixenLibJNI.Model_SetTranslation(swigCPtr, this, Vec3.getCPtr(INPUT), INPUT);
  }

  public Vec3 GetTranslation() {
    return new Vec3(VixenLibJNI.Model_GetTranslation(swigCPtr, this), true);
  }

  public void SetRotation(Quat INPUT) {
    VixenLibJNI.Model_SetRotation(swigCPtr, this, Quat.getCPtr(INPUT), INPUT);
  }

  public Quat GetRotation() {
    return new Quat(VixenLibJNI.Model_GetRotation(swigCPtr, this), true);
  }

  public void Turn(Vec3 axis, float angle) {
    VixenLibJNI.Model_Turn__SWIG_0(swigCPtr, this, Vec3.getCPtr(axis), axis, angle);
  }

  public void Turn(Quat INPUT) {
    VixenLibJNI.Model_Turn__SWIG_1(swigCPtr, this, Quat.getCPtr(INPUT), INPUT);
  }

  public void Rotate(Vec3 axis, float angle) {
    VixenLibJNI.Model_Rotate__SWIG_0(swigCPtr, this, Vec3.getCPtr(axis), axis, angle);
  }

  public void Rotate(Quat INPUT) {
    VixenLibJNI.Model_Rotate__SWIG_1(swigCPtr, this, Quat.getCPtr(INPUT), INPUT);
  }

  public void LookAt(Vec3 INPUT, float twist) {
    VixenLibJNI.Model_LookAt(swigCPtr, this, Vec3.getCPtr(INPUT), INPUT, twist);
  }

  public void Move(Vec3 arg0) {
    VixenLibJNI.Model_Move__SWIG_0(swigCPtr, this, Vec3.getCPtr(arg0), arg0);
  }

  public void Move(float x, float y, float z) {
    VixenLibJNI.Model_Move__SWIG_1(swigCPtr, this, x, y, z);
  }

  public void Translate(Vec3 INPUT) {
    VixenLibJNI.Model_Translate__SWIG_0(swigCPtr, this, Vec3.getCPtr(INPUT), INPUT);
  }

  public void Translate(float x, float y, float z) {
    VixenLibJNI.Model_Translate__SWIG_1(swigCPtr, this, x, y, z);
  }

  public void Scale(Vec3 arg0) {
    VixenLibJNI.Model_Scale__SWIG_0(swigCPtr, this, Vec3.getCPtr(arg0), arg0);
  }

  public void Scale(float x, float y, float z) {
    VixenLibJNI.Model_Scale__SWIG_1(swigCPtr, this, x, y, z);
  }

  public void Size(Vec3 INPUT) {
    VixenLibJNI.Model_Size__SWIG_0(swigCPtr, this, Vec3.getCPtr(INPUT), INPUT);
  }

  public void Size(float x, float y, float z) {
    VixenLibJNI.Model_Size__SWIG_1(swigCPtr, this, x, y, z);
  }

  public void SetTransform(Matrix src) {
    VixenLibJNI.Model_SetTransform__SWIG_0(swigCPtr, this, Matrix.getCPtr(src), src);
  }

  public void SetTransform(SWIGTYPE_p_float floatArray) {
    VixenLibJNI.Model_SetTransform__SWIG_1(swigCPtr, this, SWIGTYPE_p_float.getCPtr(floatArray));
  }

  public Matrix GetTransform() {
    long cPtr = VixenLibJNI.Model_GetTransform(swigCPtr, this);
    return (cPtr == 0) ? null : new Matrix(cPtr, false);
  }

  public void TotalTransform(Matrix output) {
    VixenLibJNI.Model_TotalTransform__SWIG_0(swigCPtr, this, Matrix.getCPtr(output), output);
  }

  public void TotalTransform(Matrix output, Model parent) {
    VixenLibJNI.Model_TotalTransform__SWIG_1(swigCPtr, this, Matrix.getCPtr(output), output, Model.getCPtr(parent), parent);
  }

  public long GetNumVtx() {
    return VixenLibJNI.Model_GetNumVtx(swigCPtr, this);
  }

  public static void setDoCulling(boolean value) {
    VixenLibJNI.Model_DoCulling_set(value);
  }

  public static boolean getDoCulling() {
    return VixenLibJNI.Model_DoCulling_get();
  }

  public static Vec3 getXAXIS() {
    return new Vec3(VixenLibJNI.Model_XAXIS_get(), false);
  }

  public static Vec3 getYAXIS() {
    return new Vec3(VixenLibJNI.Model_YAXIS_get(), false);
  }

  public static Vec3 getZAXIS() {
    return new Vec3(VixenLibJNI.Model_ZAXIS_get(), false);
  }

  public boolean Copy(SharedObj src) {
    return VixenLibJNI.Model_Copy(swigCPtr, this, SharedObj.getCPtr(src), src);
  }

  public void Render(Scene arg0) {
    VixenLibJNI.Model_Render(swigCPtr, this, Scene.getCPtr(arg0), arg0);
  }

  public final static int WORLD = VixenLibJNI.Model_WORLD_get();
  public final static int LOCAL = VixenLibJNI.Model_LOCAL_get();
  public final static int NONE = VixenLibJNI.Model_NONE_get();

  public final static int STATIC = VixenLibJNI.Model_STATIC_get();
  public final static int MORPH = VixenLibJNI.Model_MORPH_get();

  public final static int DISPLAY_NONE = VixenLibJNI.Model_DISPLAY_NONE_get();
  public final static int DISPLAY_ME = VixenLibJNI.Model_DISPLAY_ME_get();
  public final static int DISPLAY_ALL = VixenLibJNI.Model_DISPLAY_ALL_get();

}
