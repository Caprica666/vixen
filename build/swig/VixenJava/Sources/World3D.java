/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class World3D extends World {
  private long swigCPtr;

  public World3D(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.World3D_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(World3D obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_World3D(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public World3D() {
    this(VixenLibJNI.new_World3D(), true);
  }

  public static World3D Get() 
	{
		jlong ptr = VixenLibJNI.World3D_Get();
		return (World3D)  SharedObj.MakeObject(ptr, true);
	}


  public Scene GetScene(long arg0) 
	{
		jlong ptr = VixenLibJNI.World3D_GetScene__SWIG_0(swigCPtr, this, arg0);
		return (Scene)  SharedObj.MakeObject(ptr, true);
	}


  public Scene GetScene() 
	{
		jlong ptr = VixenLibJNI.World3D_GetScene__SWIG_1(swigCPtr, this);
		return (Scene)  SharedObj.MakeObject(ptr, true);
	}


  public void SetScene(Scene arg0, long arg1) {
    VixenLibJNI.World3D_SetScene__SWIG_0(swigCPtr, this, Scene.getCPtr(arg0), arg0, arg1);
  }

  public void SetScene(Scene arg0) {
    VixenLibJNI.World3D_SetScene__SWIG_1(swigCPtr, this, Scene.getCPtr(arg0), arg0);
  }

  public boolean Run(long arg0) {
    return VixenLibJNI.World3D_Run(swigCPtr, this, arg0);
  }

  public void Stop() {
    VixenLibJNI.World3D_Stop(swigCPtr, this);
  }

  public boolean OnInit() {
    return VixenLibJNI.World3D_OnInit(swigCPtr, this);
  }

  public void OnExit() {
    VixenLibJNI.World3D_OnExit(swigCPtr, this);
  }

  public void setWinRect(Box2 value) {
    VixenLibJNI.World3D_WinRect_set(swigCPtr, this, Box2.getCPtr(value), value);
  }

  public Box2 getWinRect() {
    return new Box2(VixenLibJNI.World3D_WinRect_get(swigCPtr, this), false);
  }

}
