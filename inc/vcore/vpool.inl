#pragma once

namespace Core {

inline void* BytePool::Grow(void* p, size_t nbytes)
{
	if (IsLocking())
	{
		Core::Lock	lock(m_Lock);
		return FixedLenAllocator::Grow(p, nbytes);
	}
	return FixedLenAllocator::Grow(p, nbytes);
}

inline void BytePool::FreeAll()
{
	Core::Lock	lock(m_Lock);
	ReallyFree();
	FixedLenAllocator::FreeAll();
}


} // end Core