/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class Shape extends Model {
  private long swigCPtr;

  public Shape(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.Shape_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(Shape obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_Shape(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public Shape() {
    this(VixenLibJNI.new_Shape__SWIG_0(), true);
  }

  public Shape(Shape s) {
    this(VixenLibJNI.new_Shape__SWIG_1(Shape.getCPtr(s), s), true);
  }

  public Geometry GetGeometry() 
	{
		jlong ptr = VixenLibJNI.Shape_GetGeometry(swigCPtr, this);
		return (Geometry)  SharedObj.MakeObject(ptr, true);
	}


  public void SetGeometry(Geometry arg0) {
    VixenLibJNI.Shape_SetGeometry(swigCPtr, this, Geometry.getCPtr(arg0), arg0);
  }

  public Appearance GetAppearance() 
	{
		jlong ptr = VixenLibJNI.Shape_GetAppearance(swigCPtr, this);
		return (Appearance)  SharedObj.MakeObject(ptr, true);
	}


  public void SetAppearance(Appearance arg0) {
    VixenLibJNI.Shape_SetAppearance(swigCPtr, this, Appearance.getCPtr(arg0), arg0);
  }

  public void SetHints(int hints, boolean descend) {
    VixenLibJNI.Shape_SetHints__SWIG_0(swigCPtr, this, hints, descend);
  }

  public void SetHints(int hints) {
    VixenLibJNI.Shape_SetHints__SWIG_1(swigCPtr, this, hints);
  }

  public boolean Copy(SharedObj arg0) {
    return VixenLibJNI.Shape_Copy(swigCPtr, this, SharedObj.getCPtr(arg0), arg0);
  }

  public void Render(Scene arg0) {
    VixenLibJNI.Shape_Render(swigCPtr, this, Scene.getCPtr(arg0), arg0);
  }

  public boolean CalcBound(Box3 arg0) {
    return VixenLibJNI.Shape_CalcBound(swigCPtr, this, Box3.getCPtr(arg0), arg0);
  }

  public boolean CalcSphere(Sphere arg0) {
    return VixenLibJNI.Shape_CalcSphere(swigCPtr, this, Sphere.getCPtr(arg0), arg0);
  }

}
