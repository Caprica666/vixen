/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class Scene extends SharedObj {
  private long swigCPtr;

  public Scene(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.Scene_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(Scene obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_Scene(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public Scene(SWIGTYPE_p_void r) {
    this(VixenLibJNI.new_Scene__SWIG_0(SWIGTYPE_p_void.getCPtr(r)), true);
  }

  public Scene() {
    this(VixenLibJNI.new_Scene__SWIG_1(), true);
  }

  public Scene(Scene arg0) {
    this(VixenLibJNI.new_Scene__SWIG_2(Scene.getCPtr(arg0), arg0), true);
  }

  public void Suspend() {
    VixenLibJNI.Scene_Suspend(swigCPtr, this);
  }

  public void Resume() {
    VixenLibJNI.Scene_Resume(swigCPtr, this);
  }

  public void Repaint() {
    VixenLibJNI.Scene_Repaint(swigCPtr, this);
  }

  public boolean IsSuspended() {
    return VixenLibJNI.Scene_IsSuspended(swigCPtr, this);
  }

  public void ZoomToModel(Model arg0) {
    VixenLibJNI.Scene_ZoomToModel(swigCPtr, this, Model.getCPtr(arg0), arg0);
  }

  public void ZoomToBounds(Box3 bound) {
    VixenLibJNI.Scene_ZoomToBounds(swigCPtr, this, Box3.getCPtr(bound), bound);
  }

  public void SetPriority(int p) {
    VixenLibJNI.Scene_SetPriority(swigCPtr, this, p);
  }

  public void ShowAll() {
    VixenLibJNI.Scene_ShowAll(swigCPtr, this);
  }

  public void SetViewport(Box2 arg0) {
    VixenLibJNI.Scene_SetViewport__SWIG_0(swigCPtr, this, Box2.getCPtr(arg0), arg0);
  }

  public void SetViewport(float l, float t, float r, float b) {
    VixenLibJNI.Scene_SetViewport__SWIG_1(swigCPtr, this, l, t, r, b);
  }

  public void SetViewport() {
    VixenLibJNI.Scene_SetViewport__SWIG_2(swigCPtr, this);
  }

  public void SetAutoAdjust(boolean arg0) {
    VixenLibJNI.Scene_SetAutoAdjust(swigCPtr, this, arg0);
  }

  public void SetOptions(int arg0) {
    VixenLibJNI.Scene_SetOptions(swigCPtr, this, arg0);
  }

  public void EnableOptions(int arg0) {
    VixenLibJNI.Scene_EnableOptions(swigCPtr, this, arg0);
  }

  public void DisableOptions(int arg0) {
    VixenLibJNI.Scene_DisableOptions(swigCPtr, this, arg0);
  }

  public void SetAmbient(Col4 arg0) {
    VixenLibJNI.Scene_SetAmbient__SWIG_0(swigCPtr, this, Col4.getCPtr(arg0), arg0);
  }

  public void SetAmbient(Light light) {
    VixenLibJNI.Scene_SetAmbient__SWIG_1(swigCPtr, this, Light.getCPtr(light), light);
  }

  public void SetBackColor(Col4 arg0) {
    VixenLibJNI.Scene_SetBackColor(swigCPtr, this, Col4.getCPtr(arg0), arg0);
  }

  public boolean SetColorBuffer(Bitmap arg0) {
    return VixenLibJNI.Scene_SetColorBuffer(swigCPtr, this, Bitmap.getCPtr(arg0), arg0);
  }

  public boolean SetDepthBuffer(Bitmap arg0) {
    return VixenLibJNI.Scene_SetDepthBuffer(swigCPtr, this, Bitmap.getCPtr(arg0), arg0);
  }

  public void SetCamera(Camera arg0) {
    VixenLibJNI.Scene_SetCamera(swigCPtr, this, Camera.getCPtr(arg0), arg0);
  }

  public void SetModels(Model arg0) {
    VixenLibJNI.Scene_SetModels(swigCPtr, this, Model.getCPtr(arg0), arg0);
  }

  public void SetEngines(Engine arg0) {
    VixenLibJNI.Scene_SetEngines(swigCPtr, this, Engine.getCPtr(arg0), arg0);
  }

  public void SetTimeInc(float timeinc) {
    VixenLibJNI.Scene_SetTimeInc(swigCPtr, this, timeinc);
  }

  public boolean SetWindow(long win) {
    return VixenLibJNI.Scene_SetWindow(swigCPtr, this, win);
  }

  public void Append(Scene child) {
    VixenLibJNI.Scene_Append(swigCPtr, this, Scene.getCPtr(child), child);
  }

  public Shader InstallShader(Shader arg0) {
    long cPtr = VixenLibJNI.Scene_InstallShader(swigCPtr, this, Shader.getCPtr(arg0), arg0);
    return (cPtr == 0) ? null : new Shader(cPtr, false);
  }

  public long GetWindow() {
    return VixenLibJNI.Scene_GetWindow(swigCPtr, this);
  }

  public Model GetModels() 
	{
		jlong ptr = VixenLibJNI.Scene_GetModels(swigCPtr, this);
		return (Model)  SharedObj.MakeObject(ptr, true);
	}


  public Engine GetEngines() 
	{
		jlong ptr = VixenLibJNI.Scene_GetEngines(swigCPtr, this);
		return (Engine)  SharedObj.MakeObject(ptr, true);
	}


  public Camera GetCamera() 
	{
		jlong ptr = VixenLibJNI.Scene_GetCamera(swigCPtr, this);
		return (Camera)  SharedObj.MakeObject(ptr, true);
	}


  public Bitmap GetDepthBuffer() 
	{
		jlong ptr = VixenLibJNI.Scene_GetDepthBuffer(swigCPtr, this);
		return (Bitmap)  SharedObj.MakeObject(ptr, true);
	}


  public Bitmap GetColorBuffer() 
	{
		jlong ptr = VixenLibJNI.Scene_GetColorBuffer(swigCPtr, this);
		return (Bitmap)  SharedObj.MakeObject(ptr, true);
	}


  public Box2 GetViewport() {
    return new Box2(VixenLibJNI.Scene_GetViewport(swigCPtr, this), false);
  }

  public Light GetAmbient() 
	{
		jlong ptr = VixenLibJNI.Scene_GetAmbient(swigCPtr, this);
		return (Light)  SharedObj.MakeObject(ptr, true);
	}


  public Col4 GetBackColor() {
    return new Col4(VixenLibJNI.Scene_GetBackColor(swigCPtr, this), false);
  }

  public int GetOptions() {
    return VixenLibJNI.Scene_GetOptions(swigCPtr, this);
  }

  public boolean IsAutoAdjust() {
    return VixenLibJNI.Scene_IsAutoAdjust(swigCPtr, this);
  }

  public float GetTime() {
    return VixenLibJNI.Scene_GetTime(swigCPtr, this);
  }

  public float GetTimeInc() {
    return VixenLibJNI.Scene_GetTimeInc(swigCPtr, this);
  }

  public boolean GetBound(Sphere sphere) {
    return VixenLibJNI.Scene_GetBound(swigCPtr, this, Sphere.getCPtr(sphere), sphere);
  }

  public Appearance GetPostProcess() 
	{
		jlong ptr = VixenLibJNI.Scene_GetPostProcess(swigCPtr, this);
		return (Appearance)  SharedObj.MakeObject(ptr, true);
	}


  public void SetPostProcess(Appearance arg0) {
    VixenLibJNI.Scene_SetPostProcess(swigCPtr, this, Appearance.getCPtr(arg0), arg0);
  }

  public void DoFrame() {
    VixenLibJNI.Scene_DoFrame(swigCPtr, this);
  }

  public void DoSimulation() {
    VixenLibJNI.Scene_DoSimulation(swigCPtr, this);
  }

  public void Exit() {
    VixenLibJNI.Scene_Exit(swigCPtr, this);
  }

  public static void setNumTexUnits(int value) {
    VixenLibJNI.Scene_NumTexUnits_set(value);
  }

  public static int getNumTexUnits() {
    return VixenLibJNI.Scene_NumTexUnits_get();
  }

  public static void setDeviceDepth(int value) {
    VixenLibJNI.Scene_DeviceDepth_set(value);
  }

  public static int getDeviceDepth() {
    return VixenLibJNI.Scene_DeviceDepth_get();
  }

  public static void setRenderOptions(String value) {
    VixenLibJNI.Scene_RenderOptions_set(value);
  }

  public static String getRenderOptions() {
    return VixenLibJNI.Scene_RenderOptions_get();
  }

  public boolean Copy(SharedObj arg0) {
    return VixenLibJNI.Scene_Copy(swigCPtr, this, SharedObj.getCPtr(arg0), arg0);
  }

  public void OnResize() {
    VixenLibJNI.Scene_OnResize(swigCPtr, this);
  }

  public final static int CLEARDEPTH = VixenLibJNI.Scene_CLEARDEPTH_get();
  public final static int CLEARCOLOR = VixenLibJNI.Scene_CLEARCOLOR_get();
  public final static int CLEARALL = VixenLibJNI.Scene_CLEARALL_get();
  public final static int STATESORT = VixenLibJNI.Scene_STATESORT_get();
  public final static int DOUBLEBUFFER = VixenLibJNI.Scene_DOUBLEBUFFER_get();
  public final static int FULLSCREEN = VixenLibJNI.Scene_FULLSCREEN_get();
  public final static int OCCLUSIONCULL = VixenLibJNI.Scene_OCCLUSIONCULL_get();
  public final static int REPAINT = VixenLibJNI.Scene_REPAINT_get();

}
