#pragma once

namespace Core {

inline TLSData* TLSData::Get()
{
	// Get pointer to VTlsData structure
	TLSData*	tlsPtr = (TLSData*) GetPtr();
	if (tlsPtr == NULL && !s_shutdown)
	{
		// Allocate new block structure if this is the first access. 
		Allocator* alloc = GlobalAllocator::Get();
		if (alloc)
			tlsPtr = (TLSData*) alloc->Alloc(sizeof(TLSData));
		else
			tlsPtr = (TLSData*) calloc(sizeof(TLSData), 1);
		Set(tlsPtr);
		new (tlsPtr) TLSData();
	}
	return tlsPtr;
}

#ifdef VX_NOTHREAD


inline TLSData* TLSData::GetPtr()
{
	return (TLSData *) s_tlsSlot;
}

inline void TLSData::Set(TLSData* tlsdata)
{
	s_tlsSlot = (intptr) tlsdata;
}

#else

inline void TLSData::Set(TLSData* tlsdata)
{
	pthread_setspecific((pthread_key_t) s_tlsSlot, tlsdata); 
}

inline TLSData* TLSData::GetPtr()
{
	return (TLSData*) pthread_getspecific((pthread_key_t) s_tlsSlot);
}

#endif
} // end Core

