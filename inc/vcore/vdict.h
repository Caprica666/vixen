/*!
 * @file vdict.h
 * @brief Associative array that binds an arbitrary key and value.
 *
 * @ingroup vcore
 * @author Nola Donato
 */

#pragma once

namespace Core {

#define DICT_Small		4

/*!
 * @class BaseDict
 * @param BASE	base class to inherit from
 *
 * @brief Internal base class for dictionaries.
 *
 * Provides support functions
 * for hashing keys and managing internal dictionary structure.
 * This class is templatized to allow the array to derive from any
 * subclass of BaseObj. This lets you add dictionary functionality to
 * your own classes.
 *
 * @ingroup vcore
 * @see Dict ByteArray
 */
template <class BASE> class BaseDict : public BASE
{
public:
//! Creates empty dictionary.
	BaseDict();
//! Checks if dictionary is empty.
	bool			IsEmpty() const		{ return m_Size == 0; }
//! Empties the dictionary. All keys and values are garbage collected.
	void			Empty();
//! Get allocator used for dictionary entries.
	Allocator*		GetEntryAllocator() const;
//! Establish allocator to use for dictionary entries.
	void			SetEntryAllocator(Allocator*);
//! Hashes integer key and returns bucket index.
	uint32			HashInt(intptr k) const;
//! Hashes string key and returns bucket index.
	uint32			HashStr(const TCHAR*) const;

protected:
//! Rebuilds dictionary structure as number of entries grows.
	void			Rebuild();
//! Delete a dictionary entry. 
	virtual void	DeleteEntry(List*);
//! Return hash index for dictionary entry. 
	virtual uint32	HashEntry(List*)	{ return 0; }

	Allocator*	m_EntryAlloc;
	List*		m_StaticBuckets[DICT_Small];
	int			m_Size;	        // Total entries in table
	int			m_RebuildSize;	// Rebuild table when it gets this big
	int			m_Shift;		// Shift count for hashing function
	int			m_Mask;			// Mask value for hashing function
public:
	int			m_MaxSize;		// Total buckets allocated
	List**		m_Buckets;		// -> bucket array
};

/*!
 * @class Dict
 * @param KEY	class for dictionary key used to look up items
 * @param VAL	class for value associated with each unique key
 * @param BASE	class to inherit from, must be subclass of BaseDict
 *
 * @brief Templatized hash table that can use any arbitrary
 * class as a key and another arbitrary class as its value.
 *
 * You must implement the following member functions to compare keys and hash them.
 * @code
 *	uint32	HashKey(const KEY&);
 *	bool	CompareKeys(const KEY&, const KEY&);
 * @endcode
 *
 * @ingroup vcore
 * @see Array Dict::Iter
 */
template <class KEY, class VAL, class BASE> class Dict : public BASE
{
public:
	/*!
	 * @brief Describes a dictionary entry containing both a key and a value.
	 *
	 * These entries are created and maintained by their owning dictionary.
	 * Usually, dictionary clients do not have to directly deal with entries.
	 */
	class Entry : public List
	{
	public:
	//! Create empty entry
		Entry()
		{ };
	//! Create entry with given key, empty value
		Entry(const KEY& key) : Key(key) { }
	//! Create entry with key and value
		Entry(const KEY& key, const VAL& val)
		{ Key = key; Value = val; }
		VAL&	operator=(const VAL& v)
		{ Value = v; return Value; }
	//! Retrieve the value of an entry
		operator VAL()
		{ return Value; }
		operator VAL() const
		{ return Value; }

		KEY 	Key;					//!< Key for retrieving the entry
		VAL		Value;					//!< Value associated with the key
	};

	/*!
	 * @brief Iterator to access successive elements of a dictionary.
	 *
	 * The iterator constructor initializes the iterator to begin
	 * at the first dictionary entry. Each time the \b Next function
	 * is called, a different entry is returned. The order of iteration
	 * for dictionary elements is undefined and will be unrelated to
	 * key comparison order.
	 */
	class Iter
	{
	public:
		Iter(Dict<KEY, VAL, BASE>* dict = NULL)	//!< start iterator
		{ Reset(dict); }
	/*!
	 * Retrieves the value for the next dictionary entry.
	 * @return pointer to next value or NULL if no more
	 */
		VAL*	Next()
		{
			Entry*	e = NextEntry();
			if (e)
				return &(e->Value);
			return NULL;
		}
	/*!
	 * Retrieves the next dictionary entry.
	 * @return pointer to next dictionary entry or NULL if no more
	 */
		Entry*	NextEntry()
		{
			Entry *entry;
			while (m_NextEntry == NULL)
			{
				if (m_NextIndex >= m_Dict->m_MaxSize)
					return NULL;
				m_NextEntry = (Entry*) m_Dict->m_Buckets[m_NextIndex];
				m_NextIndex++;
			}
			entry = m_NextEntry;
			m_NextEntry = (Entry*) entry->Next;
			return entry;
		}
	/*!
	 * Resets the iterator to begin at the first element.
	 * @param dict	dictionary to iterator over
	 */
		void	Reset(Dict<KEY, VAL, BASE>* dict)
		{
			m_Dict = dict;
			m_NextIndex = 0;
			m_NextEntry = NULL;
		}

	protected:
		Dict<KEY, VAL, BASE>*	m_Dict;		// Dictionary being searched
		int			m_NextIndex;			// Index of next bucket to be searched
		Entry*		m_NextEntry;			// Next entry to be searched
	};

//! Constructs empty dictionary.
	Dict() : BASE() { }
	~Dict();
//! Return dictionary entry based on key.
	Entry&			operator[](const KEY&);
	const Entry&	operator[](const KEY&) const;
//! Find value of given key.
	VAL*			Find(const KEY&) const;
//! Set value for given key.
	VAL&			Set(const KEY&, const VAL&);
//! Remove dictionary entry based on key.
	void			Remove(const KEY&);
//! Merge the source dictionary into this one.
	void			Merge(const Dict<KEY, VAL, BASE>&);
//! Compute bucket index for key.
	uint32			HashKey(const KEY&) const;
//! Compare key values.
	static bool		CompareKeys(const KEY&, const KEY&);

protected:
//! Find dictionary entry for a given key.
	virtual Entry*	FindEntry(const KEY&) const;
//! Make dictionary entry for a given key if it is not already there.
	virtual	Entry*	MakeEntry(const KEY&);
//! Find hash index for dictionary entry.
	uint32	HashEntry(List*);

	friend class Iter;
};

} // end Core
