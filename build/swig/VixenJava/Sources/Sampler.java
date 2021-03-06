/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package Vixen;

public class Sampler extends SharedObj {
  private long swigCPtr;

  public Sampler(long cPtr, boolean cMemoryOwn) {
    super(VixenLibJNI.Sampler_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  public static long getCPtr(Sampler obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        VixenLibJNI.delete_Sampler(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public Sampler() {
    this(VixenLibJNI.new_Sampler__SWIG_0(), true);
  }

  public Sampler(Sampler src) {
    this(VixenLibJNI.new_Sampler__SWIG_1(Sampler.getCPtr(src), src), true);
  }

  public Sampler(String filename) {
    this(VixenLibJNI.new_Sampler__SWIG_2(filename), true);
  }

  public Sampler(Texture arg0) {
    this(VixenLibJNI.new_Sampler__SWIG_3(Texture.getCPtr(arg0), arg0), true);
  }

  public boolean Equal(Sampler src) {
    return VixenLibJNI.Sampler_Equal(swigCPtr, this, Sampler.getCPtr(src), src);
  }

  public boolean NotEqual(Sampler src) {
    return VixenLibJNI.Sampler_NotEqual(swigCPtr, this, Sampler.getCPtr(src), src);
  }

  public Texture GetTexture() 
	{
		jlong ptr = VixenLibJNI.Sampler_GetTexture(swigCPtr, this);
		return (Texture)  SharedObj.MakeObject(ptr, true);
	}


  public void SetTexture(Texture arg0) {
    VixenLibJNI.Sampler_SetTexture(swigCPtr, this, Texture.getCPtr(arg0), arg0);
  }

  public String GetAttrName(int attr) {
    return VixenLibJNI.Sampler_GetAttrName(swigCPtr, this, attr);
  }

  public int Get(int attr) {
    return VixenLibJNI.Sampler_Get(swigCPtr, this, attr);
  }

  public void Set(int attr, int val) {
    VixenLibJNI.Sampler_Set(swigCPtr, this, attr, val);
  }

  public boolean IsMipMap() {
    return VixenLibJNI.Sampler_IsMipMap(swigCPtr, this);
  }

  public boolean HasAlpha() {
    return VixenLibJNI.Sampler_HasAlpha(swigCPtr, this);
  }

  public boolean Copy(SharedObj arg0) {
    return VixenLibJNI.Sampler_Copy(swigCPtr, this, SharedObj.getCPtr(arg0), arg0);
  }

  public final static int NONE = VixenLibJNI.Sampler_NONE_get();
  public final static int TEXTUREOP = VixenLibJNI.Sampler_TEXTUREOP_get();
  public final static int TEXCOORD = VixenLibJNI.Sampler_TEXCOORD_get();
  public final static int MINFILTER = VixenLibJNI.Sampler_MINFILTER_get();
  public final static int MAGFILTER = VixenLibJNI.Sampler_MAGFILTER_get();
  public final static int MIPMAP = VixenLibJNI.Sampler_MIPMAP_get();
  public final static int NUM_ATTRS = VixenLibJNI.Sampler_NUM_ATTRS_get();

  public final static int TEXGEN_SPHERE = VixenLibJNI.Sampler_TEXGEN_SPHERE_get();
  public final static int TEXGEN_CUBE = VixenLibJNI.Sampler_TEXGEN_CUBE_get();
  public final static int NEAREST = VixenLibJNI.Sampler_NEAREST_get();
  public final static int LINEAR = VixenLibJNI.Sampler_LINEAR_get();
  public final static int EMISSION = VixenLibJNI.Sampler_EMISSION_get();
  public final static int DIFFUSE = VixenLibJNI.Sampler_DIFFUSE_get();
  public final static int SPECULAR = VixenLibJNI.Sampler_SPECULAR_get();
  public final static int BUMP = VixenLibJNI.Sampler_BUMP_get();
  public final static int SHADER = VixenLibJNI.Sampler_SHADER_get();

}
