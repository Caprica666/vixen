#include "vixen.h"
#include "dx9/renderdx.h"

namespace Vixen
{

DXRefPtr::DXResources* DXRefPtr::s_DXObjs = (DXRefPtr::DXResources*) NULL;

DXRefPtr::DXResources::~DXResources()
{
	int	notreleased = 0;

	for (int i = 0; i < GetSize(); ++i)
		if (Release(i) == 0)
			++notreleased;
	VX_TRACE(DXRenderer::Debug, ("DirectX %d objects released: %d objects not released\n",
			   GetSize() - notreleased, notreleased));
	Empty();
}

int	DXRefPtr::DXResources::Release(int index)
{
	IUnknown* ptr;

	if ((index < 0) || (index > GetSize()))
		return -1;
	ptr = GetAt(index);
	if (ptr == NULL)
		return -1;
	if (ptr->Release() == 0)	// DX object deleted?
	{
		VX_TRACE(DXRenderer::Debug, ("DirectX object #%d released: %p", index, ptr));
		SetAt(index, NULL);		// set pointer to NULL
		return index;
	}
	return 0;			
}

int DXRefPtr::DXResources::Add(IUnknown* ptr)
{
	int index = Find(ptr);
	if (index >= 0)
		return index;
	index = GetSize();
	if (Append(ptr))
	{
		VX_TRACE(DXRenderer::Debug, ("DirectX object #%d added: %p", index, ptr));
		return index;
	}
	return -1;
}

int DXRefPtr::FindIndex(IUnknown* ptr)
{
	if (ptr == NULL)
		return 0;
	if (s_DXObjs == (DXResources*) NULL)
	{
		s_DXObjs = new DXResources;
		s_DXObjs->IncUse();
		s_DXObjs->MakeLock();
		s_DXObjs->SetAt(0, 0);
	}
	return s_DXObjs->Add(ptr);
}

int DXRefPtr::Release()
{
	int index = m_index;		// save current index

	m_index = -1;
	if (s_DXObjs == (DXResources*) NULL)
		return 0;				// no resource table
	if (index > 0)				// free from resource table
		return s_DXObjs->Release(index);
	return 0;
}


void DXRefPtr::FreeAll()
{
	if (s_DXObjs == (DXResources*) NULL)
		return;
	s_DXObjs->Delete();
	s_DXObjs = (DXResources*) NULL;
}


}