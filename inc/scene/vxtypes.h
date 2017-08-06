/****
 *
 * Basic types and constants for Visual Experience Engine
 *
 ****/
#pragma once

namespace Vixen {

/*!
 * System class identifier.
 *
 * To support serialization and extensibility, all scene manager
 * classes have a \b class \b id associated with them. This ID is really
 * an index into a global table that holds information
 * for each class. The class ID also serves as the binary file opcode
 * to instantiate a new member of the class.
 *
 * The class IDs for all of the built-in scene manager classes
 * are of the form VX_Classname (where Classname is the name of the
 * class). These are defined in ismtypes.h. If you want to add a new
 * class, define your class IDs to start at VX_NextClass. All class IDs
 * must be greater than zero and less than VX_MaxClass.
 * The binary stream I/O and class SharedObj assume the class ID
 * is a single byte (between 0 and 256)
 *
 * @see BaseObj::ClassID
 */
enum SerialID
{
	VX_Obj = 0,
	VX_World = 1,
	VX_Group = 2,
	VX_Matrix = 3,
	VX_PhongMaterial = 4,
	VX_Image = 5,
	VX_Appearance = 6,
	VX_Sampler = 7,
	VX_Engine = 8,
	VX_Scene = 9,
	VX_Array = 10,
	VX_ObjArray = 11,
	VX_IntArray = 12,
	VX_Dict = 13,
	VX_Stream = 14,
	VX_VtxArray = 15,
	VX_Bitmap = 16,
	VX_Shader = 17,
	VX_Geometry = 18,
	VX_Mesh = 19,
	VX_TriMesh = 20,
	VX_Triangles = 21,
	VX_Model = 22,
	VX_Shape = 23,
	VX_Camera = 24,
	VX_Light = 25,
	VX_DirectLight = 26,
	VX_SpotLight = 27,
	VX_Messenger = 28,
	VX_FileStream = 29,
	VX_NetStream = 30,
	VX_VtxCache = 31,
	VX_MouseEngine = 32,
	VX_DistScene = 33,
	VX_DualScene = 34,
	VX_Deformer = 35,
	VX_Loader = 36,
	VX_Arbitrator = 37,
	VX_Synchronizer = 38,
	VX_FileMessenger = 39,
	VX_World3D = 40,
	VX_Fog = 41,
	VX_FloatArray = 42,
	VX_Vec3Array = 43,
	VX_AmbientLight = 44,
	VX_Material = 45,
	
	VX_LevelOfDetail = 100,
	VX_BillBoard = 101,
	VX_Transformer = 102,
	VX_KeyFramer = 103,
	VX_KeyBlender = 104,
	VX_ModelSwitch = 105,
	VX_Switcher = 106,
	VX_Interpolator = 107,
	VX_Animator = 108,
	VX_ImageSwitch = 109,

	VX_Evaluator = 110,
	VX_Octree = 111,
	VX_Room = 112,
	VX_Wall = 113,
	VX_Portal = 114,
	VX_RoomGroup = 115,
	VX_Sprite = 116,
	VX_Skeleton = 117,
	VX_ExtModel = 118,
	VX_TextureSwitcher = 119,

	VX_BoneAnimator = 120,
	VX_Trigger = 121,
	VX_Sequencer = 122,
	VX_SimpleShape = 123,
	VX_TextGeometry = 124,
	VX_Scriptor = 125,
	VX_ColorInterp = 126,
	VX_Morph = 127,
	VX_Pose = 128,
	VX_Skin = 129,

	//	130,
	VX_TimeInterp = 131,
	VX_PoseMapper = 132,
	VX_Physics = 133,
	VX_RigidBody = 134,
	VX_RagDoll = 135,
	VX_ClothSim = 136,
	VX_ClothSkin = 137,
	VX_SkeletonMapper = 138,
	VX_MeshAnimator = 139,
	
	VX_NextClass = 200,	// numbers above 200 are not exported in 3D content

	VX_Navigator = VX_NextClass,
	VX_Picker,			// 201
	VX_RayPicker,		// 202
	VX_FrameStats,		// 203
	VX_Cursor3D,		// 204
	VX_Flyer,			// 205
	VX_ArcBall,			// 210
	VX_NavRecorder,		// 211
	VX_MediaSource,		// 212	in media library
	VX_SoundPlayer,		// 213	in media library
	VX_NamePicker,		// 221
	VX_SCAPESkin,		// 222
	VX_FaceAnimPlayer,	// 223
	VX_FaceAnimator,	// 224
	VX_FaceTracker,		// 225
	VX_BodyTracker,		// 226
	VX_KinectTracker,	// 227
	VX_OmekTracker,		// 228
};

/*
 * Property Tags used internally
 */
#define PROP_MaxScript	intptr(CLASS_(NameProp))

} // end Vixen