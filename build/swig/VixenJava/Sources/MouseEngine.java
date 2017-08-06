/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class MouseEngine extends Engine {
  private long swigCPtr;

  public MouseEngine(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.MouseEngine_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(MouseEngine obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_MouseEngine(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public MouseEngine() {
    this(VixenLibJNI.new_MouseEngine(), true);
  }

  public void OnMouse(Vec2 pos, long flags) {
    VixenLibJNI.MouseEngine_OnMouse(swigCPtr, this, Vec2.getCPtr(pos), pos, flags);
  }

}
