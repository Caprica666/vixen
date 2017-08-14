#pragma once

namespace Core
{
	
/*!
 * @class StringPool
 *
 * @brief Internal class used to manage string allocations.
 *
 * @see String Allocator
 */
//===========================================================================
class StringPool : public PoolAllocator
//===========================================================================
{
	VX_DECLARE_CLASS(StringPool);
public:
	StringPool(Allocator* pBaseAllocator = GlobalAllocator::Get());
    ~StringPool();
    
	virtual void        AllocString(String& str, size_t newLen);
    
    static StringPool*  Get()       { return s_singleton; }
    
protected:
    static StringPool*  s_singleton;
};


} // end Core
