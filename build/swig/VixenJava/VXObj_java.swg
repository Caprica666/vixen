/*
 * SWIG interface file for SharedObj
 */
%define REF_TYPEMAP(REFTYPE, JAVATYPE)
	%typemap(javatype)	REFTYPE*	"JAVATYPE"

	%typemap(javain,
			 pre="REFTYPE oref = new REFTYPE($javainput);"
			 ) REFTYPE*		"$javasclassname.getCPtr(oref)"
%enddef

namespace Vixen
{
%typemap(javaout) SharedObj, SharedObj&, SharedObj* %{
	{
		jlong ptr = $jnicall;
		return ($javaclassname)  SharedObj.MakeObject(ptr, true);
	}
%}

%typemap(javacode) SharedObj %{

public bool SameAs(SharedObj obj)
{
	if (obj.swigCPtr.Handle == swigCPtr.Handle)
		return true;
	return false;
}

public SharedObj ConvertTo(SerialID classid)
{
	if ((swigCPtr.Handle != null) &&
		IsClass((uint) classid))
		return SharedObj.MakeObject(swigCPtr.Handle, false);
	return null;
}

static public SharedObj MakeObject(jlong cptr, bool memown)
{
	SharedObj obj = new SharedObj(cptr, false);
	if (cptr == 0)
		return null;
	SerialID classid = (SerialID) obj.ClassID;
	obj.AddRef();
	switch (classid)
	{
		case SerialID.VX_IntArray:		return new IntArray(cptr, memown);
		case SerialID.VX_FloatArray:	return new FloatArray(cptr, memown);
		case SerialID.VX_ObjArray:		return new ObjArray(cptr, memown);
		case SerialID.VX_Matrix:		return new Matrix(cptr, memown);
		case SerialID.VX_PhongMaterial:	return new PhongMaterial(cptr, memown);
		case SerialID.VX_Material:		return new Material(cptr, memown);
		case SerialID.VX_Image:			return new Texture(cptr, memown);
		case SerialID.VX_Appearance:	return new Appearance(cptr, memown);
		case SerialID.VX_Engine:		return new Engine(cptr, memown);
		case SerialID.VX_Scene:			return new Scene(cptr, memown);
		case SerialID.VX_Sampler:		return new Sampler(cptr, memown);
		case SerialID.VX_VtxArray:		return new VertexArray(cptr, memown);
		case SerialID.VX_Mesh:			return new Mesh(cptr, memown);
		case SerialID.VX_TriMesh:		return new TriMesh(cptr, memown);
		case SerialID.VX_Model:			return new Model(cptr, memown);
		case SerialID.VX_ExtModel:		return new ExtModel(cptr, memown);
		case SerialID.VX_Shape:			return new Shape(cptr, memown);
		case SerialID.VX_Light:			return new Light(cptr, memown);
		case SerialID.VX_Camera:		return new Camera(cptr, memown);
		case SerialID.VX_DirectLight:	return new DirectLight(cptr, memown);
		case SerialID.VX_SpotLight:		return new SpotLight(cptr, memown);
		case SerialID.VX_World3D:		return new SharedWorld(cptr, memown);
		case SerialID.VX_Transformer:	return new Transformer(cptr, memown);
		case SerialID.VX_KeyFramer:		return new KeyFramer(cptr, memown);
		case SerialID.VX_Interpolator:	return new Interpolator(cptr, memown);
		case SerialID.VX_Trigger:		return new Trigger(cptr, memown);
		case SerialID.VX_Flyer:			return new Flyer(cptr, memown);
		case SerialID.VX_Navigator:		return new Navigator(cptr, memown);
		case SerialID.VX_Skeleton:		return new Skeleton(cptr, memown);
		case SerialID.VX_Skin:			return new Skin(cptr, memown);
		case SerialID.VX_Deformer:		return new Deformer(cptr, memown);
		case SerialID.VX_RayPicker:		return new RayPicker(cptr, memown);
		case SerialID.VX_PoseMapper:	return new PoseMapper(cptr, memown);
		case SerialID.VX_Animator:		return new Animator(cptr, memown);
		case SerialID.VX_BoneAnimator:	return new BoneAnimator(cptr, memown);
		case SerialID.VX_Scriptor:		return new Scriptor(cptr, memown);
		case SerialID.VX_Sequencer:		return new Sequencer(cptr, memown);
		case SerialID.VX_MeshAnimator:	return new MeshAnimator(cptr, memown);
		case SerialID.VX_Pose:			return new Pose(cptr, memown);
		case SerialID.VX_Morph:			return new Morph(cptr, memown);
#ifndef VIXEN_LITE
		case SerialID.VX_BodyTracker:	return new BodyTracker(cptr, memown);
		case SerialID.VX_KinectTracker:	return new KinectTracker(cptr, memown);
		case SerialID.VX_Physics:		return new Physics(cptr, memown);
		case SerialID.VX_RigidBody:		return new Transformer(cptr, memown);
		case SerialID.VX_ClothSim:		return new ClothSim(cptr, memown);
		case SerialID.VX_ClothSkin:		return new ClothSkin(cptr, memown);
		case SerialID.VX_SkeletonMapper:	return new PoseMapper(cptr, memown);
		case SerialID.VX_SoundPlayer:	return new SoundPlayer(cptr, memown);
		case SerialID.VX_MediaSource:	return new MediaSource(cptr, memown);
#endif
		default:
		{
			if (obj.IsClass((uint) SerialID.VX_Model))
				return new Model(cptr, memown);
			else if (obj.IsClass((uint) SerialID.VX_Engine))
				return new Engine(cptr, memown);
		}
		obj.Release();
		return null;
	}
}
%}
}	// end Vixen






