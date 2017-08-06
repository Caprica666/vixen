#include "vixen.h"

namespace Vixen {
using namespace Core;

void _cdecl World::Shutdown()
{
	if (s_OnlyOne != NULL)
		s_OnlyOne->Delete();
	s_OnlyOne = NULL;
	CoreExit();
}

bool World::Startup()
{
	if (!CoreInit())
		return true;
	s_OnlyOne = NULL;
	BufMessenger::s_OnlyOne = NULL;
	Core::PoolAllocator* objalloc = (Core::PoolAllocator*) Core::PoolAllocator::Get();
	objalloc->SetOptions(ALLOC_ZeroMem);
	CLASS_(BaseObj)->SetAllocator(objalloc);
	return true;
}

} // end Vixen