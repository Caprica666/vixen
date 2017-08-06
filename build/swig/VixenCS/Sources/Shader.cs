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

public class Shader : SharedObj {
  private HandleRef swigCPtr;

  internal Shader(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.Shader_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(Shader obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~Shader() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_Shader(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public Shader(int type) : this(VixenLibPINVOKE.new_Shader__SWIG_0(type), true) {
  }

  public Shader() : this(VixenLibPINVOKE.new_Shader__SWIG_1(), true) {
  }

  public bool HasSource() {
    bool ret = VixenLibPINVOKE.Shader_HasSource(swigCPtr);
    return ret;
  }

  public int GetShaderType() {
    int ret = VixenLibPINVOKE.Shader_GetShaderType(swigCPtr);
    return ret;
  }

  public void SetShaderCode(SWIGTYPE_p_void data, uint length) {
    VixenLibPINVOKE.Shader_SetShaderCode(swigCPtr, SWIGTYPE_p_void.getCPtr(data), length);
  }

  public string FileName {
    set {
      VixenLibPINVOKE.Shader_FileName_set(swigCPtr, value);
    } 
    get {
      string ret = VixenLibPINVOKE.Shader_FileName_get(swigCPtr);
      return ret;
    } 
  }

  public string Source {
    set {
      VixenLibPINVOKE.Shader_Source_set(swigCPtr, value);
    } 
    get {
      string ret = VixenLibPINVOKE.Shader_Source_get(swigCPtr);
      return ret;
    } 
  }

  public string InputDesc {
    set {
      VixenLibPINVOKE.Shader_InputDesc_set(swigCPtr, value);
    } 
    get {
      string ret = VixenLibPINVOKE.Shader_InputDesc_get(swigCPtr);
      return ret;
    } 
  }

  public string OutputDesc {
    set {
      VixenLibPINVOKE.Shader_OutputDesc_set(swigCPtr, value);
    } 
    get {
      string ret = VixenLibPINVOKE.Shader_OutputDesc_get(swigCPtr);
      return ret;
    } 
  }

  public static readonly int PIXEL = VixenLibPINVOKE.Shader_PIXEL_get();
  public static readonly int VERTEX = VixenLibPINVOKE.Shader_VERTEX_get();
  public static readonly int LIGHT = VixenLibPINVOKE.Shader_LIGHT_get();
  public static readonly int CLASS = VixenLibPINVOKE.Shader_CLASS_get();

}

}
