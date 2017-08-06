namespace Core {

/****
 *
 * Ref<T>: template class to reference a subclass of BaseObj.
 * Refs handle reference counting, making garbage collection
 * transparent.
 *
 ****/
#pragma once


//============================================================================
// BaseObj inline methods
//============================================================================
// implemented here - avail only after Allocator/ZGlobalAllocator class definitions.

/*!
 * @fn BaseObj::BaseObj()
 *
 * Constructs an empty object and associates it with an allocator.
 * The allocator owns the memory occupied by this object and will
 * be called to reclaim it when the object is destroyed.
 *
 * @return allocator pointer or NULL if not heap allocated
 *
 * @see BaseObj::GetAllocator BaseObj::GetClassAllocator Allocator
 */
// TODO: make this thread-safe
inline       BaseObj::BaseObj()	: m_alloc (t_lastAllocAt)
{
	t_lastAllocAt = NULL;
//	VX_TRACE2(BaseObj::Debug, ("%s()\n", ClassName()));
}

inline	BaseObj::~BaseObj()
{
//	VX_TRACE2(BaseObj::Debug, ("~%s()\n", ClassName()));
}

/*!
 * @fn Allocator* BaseObj::GetAllocator() const
 *
 * The object's allocator pointer always references the allocator
 * to which that object's memory belongs. When the object is
 * garbage collected, it's memory will be returned to this allocator.
 * An object which is not allocated on the heap using \b new
 * has a NULL allocator pointer.
 *
 * @return allocator pointer or NULL if not heap allocated
 *
 * @see BaseObj::GetClassAllocator
 */
inline Allocator* BaseObj::GetAllocator() const
{	return m_alloc; }

/*!
 * @fn void BaseObj::SetAllocator(Allocator* alloc)
 * @param alloc allocator which created this object
 *
 * This function is only intended for internal use during
 * object creation. It establishes which allocator the memory occupied
 * by the object belongs to.
 *
 * @internal
 * @see BaseObj::GetAllocator Class::SetAllocator
 */
inline void	BaseObj::SetAllocator(Allocator* a)
{	m_alloc = a; }

/*!
 * @fn const TCHAR* BaseObj::ClassName() const
 *
 * The class name is used to retrieve information about a class
 * that is kept in a Class object. It is also used to identify
 * the class for serialization and debugging purposes. The string
 * name of the class is the same as its C++ name and is therefor
 * case sensitive.
 *
 * @return string name of this object's class
 *
 * @see Class
 */
inline const TCHAR* BaseObj::ClassName() const
{	return GetClass()->GetName(); }

/*!
 * @fn uint32 BaseObj::ClassID() const
 *
 * The class identifier is a number that identifies the class uniquely.
 * It is usually specified at compile time and is used to identify
 * the class during serialization.
 *
 * @return numeric class identifier
 *
 * @see Class BaseObj::ClassName
 */
inline uint32 BaseObj::ClassID() const
{ return  GetClass()->GetSerialID(); }

inline void  BaseObj::operator delete (void* ptr)
{
	BaseObj*	obj = (BaseObj*) ptr;
	Allocator*	alloc = obj->GetAllocator();

	VX_TRACE(BaseObj::Debug, ("~%s() %p\n", obj->ClassName(), ptr));
	if (alloc)
		alloc->Free(ptr);
}


/*!
 * @fn void* BaseObj::operator new(size_t amount, Allocator* alloc)
 * @param amount number of bytes needed
 * @param alloc allocator to use, default to global allocator if NULL.
 *
 * Allocates memory using the allocator provided. If none is specified,
 * the default allocator for this class is used.
 *
 * The VX_DECLARE_CLASS macro automatically supplies overrides of
 * \b new for each class to control allocation behavior.
 * Because of this, subclasses of BaseObj cannot override operator new
 * to change allocation behavior. To do this, express the desired
 * allocation behavior as a Allocator subclass and use it as
 * the allocator for your class.
 *
 * @return Pointer to memory block allocated, NULL on error
 *
 * @see BaseObj::GetClassAllocator Class::SetAllocator Allocator VX_DECLARE_CLASS
 */
inline void* BaseObj::operator new(size_t amount, Allocator* alloc)
{
	if (alloc == NULL)
		alloc = ClassInfo.GetAllocator();
	void* ptr = alloc->Alloc(amount);
	t_lastAllocAt = alloc;
	return ptr;
}

/*!
 * @fn void* BaseObj::operator new(size_t amount, void* ptr)
 * @param amount number of bytes needed
 * @param ptr pointer to the already allocated memory
 *
 * This form of \b new allows an object to be constructed in
 * a designated memory area. No memory allocation is done and
 * it is the responsibility of the caller to ensure the
 * memory is large enough for construction of the object.
 *
 * @return memory pointer supplied on input
 */
inline void* BaseObj::operator new   (size_t amount, void* pWhere)
{	return pWhere; }

/*!
 * @fn void Class::SetAllocator(AllocMethod func)
 * @param func Function which retrieves a pointer to an allocator.
 *
 * This method of establishing the allocator to use for instantiating
 * members of this class permits a more dynamic way to select allocators.
 * The input function may return a variety of allocators, depending on
 * the environment in which it is called. The most common use of this
 * feature is to select the allocator for the current thread by
 * passing ThreadAllocator::Get as the function.
 *
 * If an explicit allocator pointer has been established, it supercedes
 * the function indicated here. The function will only be called if
 * an individual allocator is not selected.
 *
 * @see Class::SetAllocator(Allocator*)  Allocator Class::GetAllocator
 */
inline void	 Class::SetAllocator(AllocMethod func)
{	m_pGetAlloc = func; }

/*!
 * @fn void Class::SetAllocator(Allocator* alloc)
 * @param alloc Allocator to use for this class.
 *
 * Specifying an explicit allocator pointer causes all subsequent
 * memory allocations for objects of this class to use this allocator.
 * Any allocation function specified will be ignored. Note that any
 * thread-based or stack-based allocators cannot be used with this
 * form of the call because there is no way to vary the pointer
 * across threads or re-entrant functions. To use these allocators,
 * use the other form of the call which accepts a function to
 * produce the allocator pointer.
 *
 * @see Class::SetAllocator(AllocMethod)  Allocator Class::GetAllocator ThreadAllocator
 */
inline void	Class::SetAllocator(Allocator* alloc)
{	m_Alloc = alloc; m_pGetAlloc = NULL; }

/*!
 * @fn Allocator* Class::GetAllocator() const
 *
 * Class allocators can be specified in two ways - as a direct pointer or
 * as a function which returns the allocator pointer. If a specific allocator
 * pointer was specified, it is returned. Otherwise, the return value is computed
 * by the allocator function. This function may return a different pointer based on
 * the call environment (stack, thread, etc.)
 *
 * If neither a function nor a pointer was given, the allocator of the
 * parent class is returned. BaseObj, the common ancestor, defaults to the global allocator.
 *
 * @returns pointer to allocator to use for this class, will never be NULL
 *
 * @see Class::SetAllocator Allocator GlobalAllocator
 */
inline Allocator*	Class::GetAllocator() const
{
	if (m_pGetAlloc)
		return (*m_pGetAlloc)();
	if (m_Alloc)
		return m_Alloc;
	if (m_pBaseClass)
		return m_pBaseClass->GetAllocator();
	return GlobalAllocator::Get();
}

} // end Core