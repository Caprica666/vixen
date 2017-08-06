/****
 *
 * Inlines for SharedObj and Ref classes
 *
 ****/
#pragma once

namespace Vixen {

inline Property&	Property::operator=(const Property& p)
{
	return *this;
}

/*!
 * @fn void Delete(SharedObj* obj)
 *
 * Replaces the \b delete operator for deleting a SharedObj.
 * This is necessary because the use count must be
 * checked before \b delete is called. You won't ever
 * need this function if you use Refs
 *
 * @see Ref
 */
inline void Delete(SharedObj* obj)
	{ if (obj) obj->Delete(); }


/*!
 * @fn void SharedObj::SetID(uint32 id)
 *
 * In the distributed environment, each object is assigned a
 * unique identifier (not a pointer) to identify it in the
 * networked environment. This identifier is assigned when
 * the object is attached to an output stream. All transactions
 * logged to the stream use this handle to identify the object.
 * If the object is distributed, this handle serves as a unique
 * identifier for it throughout the system.
 *
 * @see SharedObj::Create BaseObj::ClassID Messenger::Attach Messenger::Create
 */
inline void SharedObj::SetID(uint32 id)
	{ m_ID = id; }

inline uint32 SharedObj::GetID() const
	{ return m_ID; }

/*!
 * @fn void SharedObj::SetFlags(uint32 flags) const
 * @param flags	bit flags to set
 *
 * Each object has a set of bit flags used for notification.
 * Vixen internally uses two of these flags on all objects,
 * but individual subclasses of SharedObj may use more of these flags.
 * The base object uses these:
 * - SharedObj::NOFREE		marks an object as not freeable
 * - SharedObj::CHANGED		marks an object as having been changed
 * - SharedObj::INACTIVE	marks an object as inactive (not displaying or executing)
 * - SharedObj::SHARED		marks an object as shared with remote clients
 * - SharedObj::GLOBAL		marks an object as shared and global to remote clients
 *
 * These flags may also be used to update the object flags in user
 * applications as a form of notification for read-only operations.
 * Subclasses which override the rendering functions can use these
 * flags without violating the \b const and be ensured it is done
 * in a thread-safe way.
 *
 * @see SharedObj::SetChanged  SharedObj::ClearFlags SharedObj::IsSet SharedObj::IsGlobal
 */
inline void SharedObj::SetFlags(uint32 f) const
{
	Core::InterlockOr(&m_Flags, f);
}

inline uint32	SharedObj::GetFlags() const
	{ return m_Flags; }


/*!
 * @fn void SharedObj::ClearFlags(uint32 flags) const
 * @param flags	bit flags to clear
 *
 * Each object has a set of bit flags used for notification.
 * Vixen internally uses two of these flags on all objects,
 * but individual subclasses of SharedObj may use more of these flags.
 *
 * These flags may also be used to update the object flags in user
 * applications as a form of notification for read-only operations.
 * Subclasses which override the rendering functions can use these
 * flags without violating the \b const and be ensured it is done
 * in a thread-safe way.
 *
 * @see SharedObj::SetChanged  SharedObj::SetFlags SharedObj::IsSet SharedObj::IsGlobal
 */
inline void SharedObj::ClearFlags(uint32 f) const
{
	Core::InterlockAnd(&m_Flags, ~f);
}

/*!
 * @fn bool SharedObj::IsSet(uint32 f) const
 * @param f	bit flags to check
 *
 * Each object has a set of bit flags used for notification.
 * Vixen internally uses two of these flags on all objects,
 * but individual subclasses of SharedObj may use more of these flags.
 * The base object uses these:
 * - SharedObj::NOFREE		marks an object as not freeable
 * - SharedObj::CHANGED		marks an object as having been changed
 * - SharedObj::INACTIVE	marks an object as inactive (not displaying or executing)
 * - SharedObj::SHARED		marks an object as shared with remote clients
 * - SharedObj::GLOBAL		marks an object as shared and global to remote clients
 *
 * @see SharedObj::SetChanged  SharedObj::ClearFlags SharedObj::HasChanged SharedObj::IsGlobal
 */
inline bool SharedObj::IsSet(uint32 f) const
{
	return (m_Flags & f) == f;
}
 
/*!
 * @fn int32 SharedObj::IsGlobal() const
 *
 * A distributed object exists on multiple machines and is
 * kept synchronized by propagating updates across
 * the network using Vixen streams. An object which is \b shared
 * but not global will return 0 for this call.
 *
 * All updates to a global object are made immediately to the local
 * version of the object and are also logged to the output
 * stream of the current scene. At the end of each frame,
 * the transactions for all global objects are sent to remote
 * processors. Updates made to these object remotely are
 * read by the scene's input stream and applied locally 
 * before the next frame begins.
 *
 * @return GLOBAL if object is global, else zero
 *
 * @see Messenger Messenger::Attach SharedObj::GetID SharedObj::ShareClass SharedObj::SetFlags
 */
inline int32 SharedObj::IsGlobal() const
	{ return (m_Flags & GLOBAL); }

inline int32 SharedObj::IsShared() const
	{ return (m_Flags & (GLOBAL | SHARED)); }

inline SharedObj& SharedObj::operator=(const SharedObj& src)
	{ Copy(&src); return *this; }

/*!
 * @fn void SharedObj::SetChanged(bool flag) const
 *
 * Basic change notification mechanism. The lowest order
 * bit in the object flags, \b CHANGED, is reserved
 * for handling object updates.	It is up to the individual
 * object API as to what constitutes an update and when
 * the flag is cleared.
 *
 * @see SharedObj::SetFlags SharedObj::HasChanged
 */
inline bool SharedObj::HasChanged() const
	{ return IsSet(CHANGED); }

inline void SharedObj::SetChanged(bool flag) const
{
	if (flag) SetFlags(CHANGED);
	else ClearFlags(CHANGED);
}

inline bool	SharedObj::IsActive() const
	{ return !IsSet(INACTIVE); }
 
} // end Vixen