/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class TriggerEvent extends Event {
  private long swigCPtr;

  public TriggerEvent(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.TriggerEvent_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(TriggerEvent obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_TriggerEvent(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public TriggerEvent(int code) {
    this(VixenLibJNI.new_TriggerEvent__SWIG_0(code), true);
  }

  public TriggerEvent() {
    this(VixenLibJNI.new_TriggerEvent__SWIG_1(), true);
  }

  public TriggerEvent(TriggerEvent src) {
    this(VixenLibJNI.new_TriggerEvent__SWIG_2(TriggerEvent.getCPtr(src), src), true);
  }

  public Event Copy(Event arg0) {
    return new Event(VixenLibJNI.TriggerEvent_Copy(swigCPtr, this, Event.getCPtr(arg0), arg0), false);
  }

  public void setTarget(ObjRef value) {
ObjRef oref = new ObjRef(value);
    {
      VixenLibJNI.TriggerEvent_Target_set(swigCPtr, this, $javasclassname.getCPtr(oref), value);
    }
  }

  public ObjRef getTarget() {
    long cPtr = VixenLibJNI.TriggerEvent_Target_get(swigCPtr, this);
    return (cPtr == 0) ? null : new ObjRef(cPtr, false);
  }

  public void setCollider(ObjRef value) {
ObjRef oref = new ObjRef(value);
    {
      VixenLibJNI.TriggerEvent_Collider_set(swigCPtr, this, $javasclassname.getCPtr(oref), value);
    }
  }

  public ObjRef getCollider() {
    long cPtr = VixenLibJNI.TriggerEvent_Collider_get(swigCPtr, this);
    return (cPtr == 0) ? null : new ObjRef(cPtr, false);
  }

  public void setCollidePos(Vec3 value) {
    VixenLibJNI.TriggerEvent_CollidePos_set(swigCPtr, this, Vec3.getCPtr(value), value);
  }

  public Vec3 getCollidePos() {
    return new Vec3(VixenLibJNI.TriggerEvent_CollidePos_get(swigCPtr, this), false);
  }

}
