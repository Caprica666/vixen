#ifdef _AFXDLL
	#include "vxmfc.h"
	#define	WORLD	MFC::Demo
#else
	#include "vixen.h"
	#include "win32/vxwinworld.h"
	#define	WORLD	WinWorld
#endif

#include "FBXReader.h"
using namespace Vixen;

class FBXWorld : public WORLD
{
public:
	FBXWorld(): WORLD()  {  }
	virtual	bool	OnInit();
};


bool FBXWorld::OnInit()
{
	WinRect.Set(0, 640, 0, 480);
	Skeleton::Debug = 2;
	Skin::Debug = 2;
	Pose::Debug = 2;
	if (WORLD::OnInit())
	{
		FileLoader* loader = GetLoader();

		VX_ASSERT(loader);
//		FBX::FBXReader::DoDeformers = false;
//		FBX::FBXReader::DoGeometry = false;
//		FBX::FBXReader::DoAnimation = false;
		loader->SetFileFunc(TEXT("fbx"), &FBX::ReadFBXFile, Event::LOAD_SCENE);
		loader->SetFileFunc(TEXT("bvh"), &BVHLoader::ReadAnim, Event::LOAD_SCENE);
		return true;
	}
	return false;
}

VIXEN_MAKEWORLD(FBXWorld);
MFC::App	theMFCApp;



