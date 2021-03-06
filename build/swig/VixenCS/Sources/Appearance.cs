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

public class Appearance : ObjArray {
  private HandleRef swigCPtr;

  internal Appearance(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.Appearance_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(Appearance obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~Appearance() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_Appearance(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public Appearance(int numsamplers) : this(VixenLibPINVOKE.new_Appearance__SWIG_0(numsamplers), true) {
  }

  public Appearance() : this(VixenLibPINVOKE.new_Appearance__SWIG_1(), true) {
  }

  public Appearance(string filename) : this(VixenLibPINVOKE.new_Appearance__SWIG_2(filename), true) {
  }

  public Appearance(Appearance arg0) : this(VixenLibPINVOKE.new_Appearance__SWIG_3(Appearance.getCPtr(arg0)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public Appearance(Col4 arg0) : this(VixenLibPINVOKE.new_Appearance__SWIG_4(Col4.getCPtr(arg0)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public virtual bool Equal(Appearance src) {
    bool ret = VixenLibPINVOKE.Appearance_Equal(swigCPtr, Appearance.getCPtr(src));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool NotEqual(Appearance src) {
    bool ret = VixenLibPINVOKE.Appearance_NotEqual(swigCPtr, Appearance.getCPtr(src));
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public virtual bool SetSampler(int index, Sampler app) {
    bool ret = VixenLibPINVOKE.Appearance_SetSampler(swigCPtr, index, Sampler.getCPtr(app));
    return ret;
  }

  public Sampler GetSampler(int index) 
	{
		IntPtr cPtr = VixenLibPINVOKE.Appearance_GetSampler(swigCPtr, index);
		return (Sampler)  SharedObj.MakeObject(cPtr, true);
	}


  public virtual void SetVertexShader(Shader arg0) {
    VixenLibPINVOKE.Appearance_SetVertexShader(swigCPtr, Shader.getCPtr(arg0));
  }

  public Shader GetVertexShader() {
    IntPtr cPtr = VixenLibPINVOKE.Appearance_GetVertexShader(swigCPtr);
    Shader ret = (cPtr == IntPtr.Zero) ? null : new Shader(cPtr, false);
    return ret;
  }

  public int Get(int attr) {
    int ret = VixenLibPINVOKE.Appearance_Get(swigCPtr, attr);
    return ret;
  }

  public virtual void Set(int attr, int val) {
    VixenLibPINVOKE.Appearance_Set(swigCPtr, attr, val);
  }

  public string GetAttrName(int attr) {
    string ret = VixenLibPINVOKE.Appearance_GetAttrName(swigCPtr, attr);
    return ret;
  }

  public static void Apply(Model root, int attr, int val) {
    VixenLibPINVOKE.Appearance_Apply(Model.getCPtr(root), attr, val);
  }

  public static bool DoLighting {
    set {
      VixenLibPINVOKE.Appearance_DoLighting_set(value);
    } 
    get {
      bool ret = VixenLibPINVOKE.Appearance_DoLighting_get();
      return ret;
    } 
  }

  public Material Material {
    set {
      VixenLibPINVOKE.Appearance_Material_set(swigCPtr, Material.getCPtr(value));
    } 
	get
	{
		IntPtr cPtr = VixenLibPINVOKE.Appearance_Material_get(swigCPtr);
		return (Material) SharedObj.MakeObject(cPtr, false);
	}

  }

  public Shader PixelShader {
    set {
      VixenLibPINVOKE.Appearance_PixelShader_set(swigCPtr, Shader.getCPtr(value));
    } 
    get {
      IntPtr cPtr = VixenLibPINVOKE.Appearance_PixelShader_get(swigCPtr);
      Shader ret = (cPtr == IntPtr.Zero) ? null : new Shader(cPtr, false);
      return ret;
    } 
  }

  public int NumSamplers {
    set {
      VixenLibPINVOKE.Appearance_NumSamplers_set(swigCPtr, value);
    } 
    get {
      int ret = VixenLibPINVOKE.Appearance_NumSamplers_get(swigCPtr);
      return ret;
    } 
  }

  public static readonly int NONE = VixenLibPINVOKE.Appearance_NONE_get();
  public static readonly int CULLING = VixenLibPINVOKE.Appearance_CULLING_get();
  public static readonly int LIGHTING = VixenLibPINVOKE.Appearance_LIGHTING_get();
  public static readonly int SHADING = VixenLibPINVOKE.Appearance_SHADING_get();
  public static readonly int ZBUFFER = VixenLibPINVOKE.Appearance_ZBUFFER_get();
  public static readonly int TRANSPARENCY = VixenLibPINVOKE.Appearance_TRANSPARENCY_get();
  public static readonly int NUM_ATTRS = VixenLibPINVOKE.Appearance_NUM_ATTRS_get();

  public static readonly int SMOOTH = VixenLibPINVOKE.Appearance_SMOOTH_get();
  public static readonly int WIRE = VixenLibPINVOKE.Appearance_WIRE_get();
  public static readonly int POINTS = VixenLibPINVOKE.Appearance_POINTS_get();

}

}
