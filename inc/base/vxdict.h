/*!
 * @file vxdict.h
 *
 * @brief Defines templatized thread-safe hash tables that permit
 * keys and values to be arbitrary classes.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxarray.h vdict.h
 */

#pragma once

namespace Vixen {

/*!
 * @class BaseDict
 * @brief shared dictionary base object.
 */
class BaseDict : public Core::BaseDict< Vixen::SharedObj >
{
public:
	VX_DECLARE_CLASS(BaseDict);
};

/*!
 * @class Dictionary
 * @brief Templatized hash table that can use any arbitrary
 * class as a key and another arbitrary class as its value.
 * @param KEY	class for key used to access items
 * @param VAL	class for value associated with each unique key
 *
 * It is capable of locking around dictionary access and update for thread-safety.
 * You must implement the following member functions to compare keys and hash them.
 * They define how your dictionary implements hashing and how the key classes
 * are compared for equality. There are no default implementations for these.
 * @code
 *	uint32	HashKey(const KEY&);
 *	bool	CompareKeys(const KEY&, const KEY&);
 * @endcode
 *
 * @see Array Dict::Iter
 */
template <class KEY, class VAL> class Dictionary : public Core::Dict<KEY, VAL, Vixen::BaseDict>
{
public:
	Dictionary() : Core::Dict<KEY, VAL, BaseDict>() { }
#ifndef VX_NOTHREAD
	typename Core::Dict<KEY, VAL, BaseDict>::Entry&		operator[](const KEY& k)
		{ ObjectLock lock(this); return Core::Dict<KEY, VAL, BaseDict>::operator[](k); }
	const typename Core::Dict<KEY, VAL, BaseDict>::Entry&	operator[](const KEY& k) const
		{ ObjectLock lock(this); return Dict<KEY, VAL, BaseDict>::operator[](k); }
	VAL*	Find(const KEY& k) const
		{ ObjectLock lock(this); return Core::Dict<KEY, VAL, BaseDict>::Find(k); }
	VAL&	Set(const KEY& k, const VAL& v)
		{ ObjectLock lock(this); return Core::Dict<KEY, VAL, BaseDict>::Set(k, v); }
	void	Remove(const KEY& k)
		{ ObjectLock lock(this); Core::Dict<KEY, VAL, BaseDict>::Remove(k); }
	void	Merge(const Core::Dict<KEY, VAL, BaseDict>& srcdict)
		{ ObjectLock lock(this); Core::Dict<KEY, VAL, BaseDict>::Merge(srcdict); }
#endif
};

/*!
 * @class DictIter
 * @brief Iterator to access successive elements of a dictionary.
 * @param KEY	class for key used to access items
 * @param VAL	class for value associated with each unique key
 *
 * The iterator constructor initializes the iterator to begin
 * at the first dictionary entry. Each time the \b Next function
 * is called, a different entry is returned. The order of iteration
 * for dictionary elements is undefined and will be unrelated to
 * key comparison order.
 *
 * This dictionary iterator is thread-safe and will lock the entire
 * dictionary around iteration if locking is enabled.
 *
 * @see Dict::Iter Dictionary
 */
template <class KEY, class VAL> class DictIter : public Core::Dict<KEY, VAL, Vixen::BaseDict>::Iter
{
public:
#ifdef VX_NOTHREAD
    DictIter(Core::Dict<KEY, VAL, Vixen::BaseDict>* dict) : Core::Dict<KEY, VAL, Vixen::BaseDict>::Iter(dict) { }
#else
    DictIter(Core::Dict<KEY, VAL, Vixen::BaseDict>* dict) : Core::Dict<KEY, VAL, Vixen::BaseDict>::Iter(dict)
	{
		if (dict)
			dict->Lock();
	}
    ~DictIter()
	{
		if (Core::Dict<KEY, VAL, Vixen::BaseDict>::Iter::m_Dict)
			Core::Dict<KEY, VAL, Vixen::BaseDict>::Iter::m_Dict->Unlock();
	}
	void Reset(const Core::Dict<KEY, VAL, BaseDict>& dict)
	{
		if ((Core::Dict<KEY, VAL, Vixen::BaseDict>::Iter::m_Dict != dict) && !Core::Dict<KEY, VAL, Vixen::BaseDict>::Iter::m_Dict.IsNull())
			Core::Dict<KEY, VAL, Vixen::BaseDict>::Iter::m_Dict->Unlock();
		if (dict)
			dict->Lock();
		Core::Dict<KEY, VAL, Vixen::BaseDict>::Iter::Reset(dict);
	}
#endif
};

namespace Core
{
template <> inline bool Dict<NameProp, ObjRef, Vixen::BaseDict>::CompareKeys(const NameProp& knew, const NameProp& kdict)
{
//	return STRCASECMP(knew, kdict) == 0;
	return String(knew).CompareNoCase(kdict) == 0;
}

template <> inline uint32 Dict<NameProp, ObjRef, Vixen::BaseDict>::HashKey(const NameProp& np) const
{
	return HashStr((const TCHAR*) np);
}

}


/*!
 * @class NameDict
 * @brief  Describes a named collection of homogenous items.
 * @param ITEM	class for item associated with each unique name
 *
 * It is capable of locking around dictionary access and update for thread-safety.
 * The class is optimized for retrieval of an individual item
 * given its exact string name. You can also scan the entire dictionary
 * to obtain all items with a given name but this is somewhat slower.
 * An asterisk may be used as a wildcard character to match part of a name.
 *
 * @see Dictionary NameProp
 */
template <class ITEM> class NameDict : public Dictionary<NameProp, ITEM>
{
public:
// find single entry with wildcard match
	ITEM*			FindWild(const TCHAR* name) const;
// find single entry with exact match
	ITEM*			Find(const TCHAR* name) const;
// Find multiple entries using wildcard match.
	Array<ITEM>*	FindAll(const TCHAR* name) const;
};

/*!
 * @class NameIter
 * @param ITEM	class for item associated with each entry.
 *
 * @brief Iterator that can selectively return only those dictionary entries
 * which match a given search string.
 *
 * The wildcard "*" will match any sequence of characters in a dictionary key.
 * Putting an asterisk at the beginning of the search string will
 * match if the dict entry ends with that string. Similarly, putting
 * an asterisk at the end of the search string will match if the dictionary
 * key starts with that string. You can put an asterisk at both ends
 * to match if the entry contains the search string.
 *
 * This dictionary iterator is thread-safe and will lock the entire
 * dictionary around iteration if locking is enabled.
 *
 * @see NameDict NameProp
 */
template <class ITEM> class NameIter : public DictIter<NameProp, ITEM>
{
public:
	NameIter(Dictionary<NameProp, ObjRef>* dict, const TCHAR* searchkey = NULL)
	: DictIter<NameProp, ITEM>(dict), m_SearchKey(searchkey) { }

	bool	CompareWild(const NameProp& kdict) const;
	Dictionary<NameProp, ObjRef>::Entry* NextWild();
protected:
	const TCHAR*	m_SearchKey;
};

#include "vxdict.inl"

/*!
  * @class NameTable
  * @brief Dictionary of objects indexed by their string name
  */
class NameTable : public NameDict<ObjRef>
{
public:
/*!
 * Performs a wildcard search for a objects based on their name.
 * The wildcard "*" will match any sequence of characters in the object name.
 * Putting an asterisk at the beginning of the name search string will
 * match object names that end with the search string. Similarly, putting
 * an asterisk at the beginning will match object names that end
 * with the searcdh string. You can put an asterisk at both ends
 * to match the object names which contain the search string.
 *
 * @return array of objects whose names match the search string, NULL if no matches
 *
 * @see NameDict::FindWild NameIter
 */
	ObjArray*	FindAll(const TCHAR* name) const
	{
		NameIter<ObjRef> iter((NameTable*) this, name);
		Entry*	e;
		ObjArray* arr = new ObjArray;

		while (e = (Entry*) iter.NextWild())
		{
			SharedObj*	obj = e->Value;
			if (obj)
				arr->Append(obj);
		}
		if (arr->GetSize() == 0)
		{
			arr->Delete();
			return NULL;
		}
		return arr;
	}

	void	Merge(const Dictionary<NameProp, ObjRef>& srcdict)
	{
		NameIter<ObjRef> iter((Dictionary<NameProp, ObjRef>*) &srcdict);
		Entry*	src;

		while (src = (Entry*) iter.NextEntry())
			Set(src->Key, src->Value);
	}
};

} // end Vixen
