
#include "vcore/vcore.h"

namespace Vixen {
namespace Core {

/*!
 * @fn bool CoreInit()
 *
 * Initializes the core features. After this call, allocators and run-time typing
 * will be available. It should be called before instantiating anything derived
 * from BaseObj.
 *
 * @relates BaseObj
 */

// char    galloc[sizeof(GlobalAllocator)];

bool CoreInit()
{
//	if (GlobalAllocator::Get())
//		return false;
//
//	new (galloc) GlobalAllocator;
//    String::CoreInit();

#ifndef VX_NOTHREAD
// hack.hack.hack !!!!!!!!!!!!!!!!!!!!!!!!!!!
//	Class* csec = CLASS_(CritSec);
//    csec->SetAllocator(TLSData::Get()->GetLockPool());
#endif
	
	Class::LinkClassHierarchy();
	
	return true;
}

void _cdecl CoreExit()
{
//	if (GlobalAllocator::Get() == NULL)
//		return;
    
//    String::CoreFini();
    
//    //	Don't call free(GlobalAllocator::s_ptheOneAndOnly); this object was created with placement new
//    GlobalAllocator::Get()->~GlobalAllocator();
}

}	// end Core
}	// end Vixen
