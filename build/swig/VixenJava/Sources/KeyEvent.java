/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class KeyEvent extends Event {
  private long swigCPtr;

  public KeyEvent(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.KeyEvent_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(KeyEvent obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_KeyEvent(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public KeyEvent() {
    this(VixenLibJNI.new_KeyEvent__SWIG_0(), true);
  }

  public KeyEvent(Event src) {
    this(VixenLibJNI.new_KeyEvent__SWIG_1(Event.getCPtr(src), src), true);
  }

  public Event Copy(Event arg0) {
    return new Event(VixenLibJNI.KeyEvent_Copy(swigCPtr, this, Event.getCPtr(arg0), arg0), false);
  }

  public void setKeyCode(int value) {
    VixenLibJNI.KeyEvent_KeyCode_set(swigCPtr, this, value);
  }

  public int getKeyCode() {
    return VixenLibJNI.KeyEvent_KeyCode_get(swigCPtr, this);
  }

  public void setKeyFlags(int value) {
    VixenLibJNI.KeyEvent_KeyFlags_set(swigCPtr, this, value);
  }

  public int getKeyFlags() {
    return VixenLibJNI.KeyEvent_KeyFlags_get(swigCPtr, this);
  }

  public final static int K_ESCAPE = VixenLibJNI.KeyEvent_K_ESCAPE_get();
  public final static int K_END = VixenLibJNI.KeyEvent_K_END_get();
  public final static int K_HOME = VixenLibJNI.KeyEvent_K_HOME_get();
  public final static int K_LEFT = VixenLibJNI.KeyEvent_K_LEFT_get();
  public final static int K_UP = VixenLibJNI.KeyEvent_K_UP_get();
  public final static int K_RIGHT = VixenLibJNI.KeyEvent_K_RIGHT_get();
  public final static int K_DOWN = VixenLibJNI.KeyEvent_K_DOWN_get();
  public final static int K_INSERT = VixenLibJNI.KeyEvent_K_INSERT_get();
  public final static int K_DELETE = VixenLibJNI.KeyEvent_K_DELETE_get();
  public final static int K_PAGEUP = VixenLibJNI.KeyEvent_K_PAGEUP_get();
  public final static int K_PAGEDOWN = VixenLibJNI.KeyEvent_K_PAGEDOWN_get();
  public final static int K_F1 = VixenLibJNI.KeyEvent_K_F1_get();
  public final static int K_F2 = VixenLibJNI.KeyEvent_K_F2_get();
  public final static int K_F3 = VixenLibJNI.KeyEvent_K_F3_get();
  public final static int K_F4 = VixenLibJNI.KeyEvent_K_F4_get();
  public final static int K_F5 = VixenLibJNI.KeyEvent_K_F5_get();
  public final static int K_F6 = VixenLibJNI.KeyEvent_K_F6_get();
  public final static int K_F7 = VixenLibJNI.KeyEvent_K_F7_get();
  public final static int K_F8 = VixenLibJNI.KeyEvent_K_F8_get();
  public final static int K_F9 = VixenLibJNI.KeyEvent_K_F9_get();
  public final static int K_F10 = VixenLibJNI.KeyEvent_K_F10_get();
  public final static int K_F11 = VixenLibJNI.KeyEvent_K_F11_get();
  public final static int K_F12 = VixenLibJNI.KeyEvent_K_F12_get();
  public final static int K_ADD = VixenLibJNI.KeyEvent_K_ADD_get();
  public final static int K_SUBTRACT = VixenLibJNI.KeyEvent_K_SUBTRACT_get();
  public final static int K_CONTROL = VixenLibJNI.KeyEvent_K_CONTROL_get();

  public final static int ALT = VixenLibJNI.KeyEvent_ALT_get();
  public final static int UP = VixenLibJNI.KeyEvent_UP_get();
  public final static int SHIFT = VixenLibJNI.KeyEvent_SHIFT_get();
  public final static int CONTROL = VixenLibJNI.KeyEvent_CONTROL_get();
  public final static int RETURN = VixenLibJNI.KeyEvent_RETURN_get();
  public final static int REPEAT = VixenLibJNI.KeyEvent_REPEAT_get();

}
