#pragma once

WINBASEAPI LPVOID WINAPI ::TlsGetValue(DWORD dwTlsIndex);
WINBASEAPI BOOL WINAPI ::TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue);

namespace Core {

/*!
 * @relates TLSData
 * Creates a new thread local storage data area.
 * C-style routine to call from assembly language.  This needs to be
 * a globally-defined "C" function to ensure that the ensuing code
 * is really generated as inline assembly.
 */
inline TLSData* TLSData::Get()
{
	// Get pointer to TLSData structure
	TLSData*	tlsPtr = (TLSData*) ::TlsGetValue((DWORD) s_tlsSlot);
	if (tlsPtr == NULL && !s_shutdown)
	{
		tlsPtr = (TLSData*) calloc(sizeof(TLSData), 1);
		::TlsSetValue((DWORD) s_tlsSlot, tlsPtr);
		new (tlsPtr) TLSData();
	}
	return tlsPtr;
}

inline void TLSData::Set(TLSData* tlsdata)
{
	::TlsSetValue((DWORD) s_tlsSlot, tlsdata); 
}

inline TLSData* TLSData::GetPtr()
{
	return (TLSData*) ::TlsGetValue((DWORD) s_tlsSlot); 
}


} // end Core
