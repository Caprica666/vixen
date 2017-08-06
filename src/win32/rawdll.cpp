#include "vixen.h"
using namespace Vixen;

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
		World::Startup();
		break;

		case DLL_PROCESS_DETACH:
		World::Shutdown();
		break;
	}
	return TRUE;
}

