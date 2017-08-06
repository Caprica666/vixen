namespace Core {

/*
 * When there are this many entries per bucket, on average, rebuild
 * the hash table to make it larger.
 */
#define DICT_RebuildSize	3

template <class KEY, class VAL, class BASE>
	inline Dict<KEY, VAL, BASE>::~Dict() { BASE::Empty(); }

/*!
 * @fn void BaseDict::DeleteEntry(List* entry)
 * @param entry pointer to dictionary entry to delete
 *
 * Called internally before a dictionary entry is deleted.
 * Subclasses may override to change or intercept this operation.
 * The default implementation just calls system \b delete.
 *
 *@see Dict::Remove
 */
template <class BASE> void BaseDict<BASE>::DeleteEntry(List* entry)
{
	delete entry;
} 

/*!
 * @fn void Allocator* ByteArray::GetElemAllocator() const
 * @return pointer to the element allocator, never NULL
 *
 * The default allocation method uses the same allocator as
 * the parent class \b BASE of this ByteArray.
 * Normally this is the global allocator but, if your application changes
 * the default allocator for this parent class,
 * the default element allocator changes accordingly.
 *
 * @see Allocator ByteArray::GetElemAllocator BaseObj GlobalAllocator
 */
template <class BASE> inline Allocator* BaseDict<BASE>::GetEntryAllocator() const
{
	if (m_EntryAlloc)
		return m_EntryAlloc;
	return BASE::GetClassAllocator();
}

/*!
 * @fn void ByteArray::SetElemAllocator(Allocator* alloc)
 * @param alloc Allocator to use for array element data storage.
 *
 * The default allocation method uses the global allocator.
 *
 * @see Allocator ByteArray::GetElemAllocator
 */
template <class BASE> inline void BaseDict<BASE>::SetEntryAllocator(Allocator* alloc)
{
	if (alloc)
	   { VX_ASSERT(m_MaxSize == 0); }
	m_EntryAlloc = alloc;
}

/*!
 * @fn void Dict::HashEntry(List* entry)
 * @param entry pointer to dictionary entry to hash
 *
 * Called internally when rebuilding the dictionary to get
 * the new hash index for an existing element.
 * Subclasses may override to change or intercept this operation.
 * The default implementation just calls \b HashKey.
 *
 *@see Dict::HashKey BaseDict::HashStr
 */
template <class KEY, class VAL, class BASE>
	uint32 Dict<KEY, VAL, BASE>::HashEntry(List* entry)
{
	Entry* e = (Entry*) entry;
	return HashKey(e->Key);
} 

/*!
 * @fn void Dict::Remove(const KEY& key)
 * @param key key for entry to remove
 *
 * Finds the dictionary entry with the given key and removes it.
 * The value associated with the key is also deleted.
 *
 * @see Dict::Find Dict::Set BaseDict::DeleteEntry
 */
template <class KEY, class VAL, class BASE>
	void Dict<KEY, VAL, BASE>::Remove(const KEY& key)
{
	Entry*	prev;

    uint32 index = HashKey(key) & BASE::m_Mask;
    Entry* entry = (Entry*) BASE::m_Buckets[index];	// search all entries

	if (entry == NULL)							// no entries in bucket
		return;
/*
 * First check if the first entry is the one we want to kill
 */
	if (CompareKeys(key, entry->Key))
	{
		BASE::m_Buckets[index] = entry->Next;
	   	BASE::m_Size--;
		entry->Next = NULL;
		Dict<KEY, VAL, BASE>::DeleteEntry(entry);
		return;
}
/*
 * Now check the other entries in the bucket. If we
 * find a key that matches, we delete the entry
 * (and do not look for more)
 */
	while (prev = entry)
	{
		entry = (Entry*) prev->Next;
		if (entry == NULL)
			return;
		if (CompareKeys(key, entry->Key))
		{
			prev->Next = entry->Next;
	    	BASE::m_Size--;
			entry->Next = NULL;
			Dict<KEY, VAL, BASE>::DeleteEntry(entry);
			return;
		}
	}
}

/*!
 * @fn Entry* Dict::FindEntry(const KEY& key) const
 * @param key key for entry to find
 *
 * Finds the dictionary entry associated with the given key.
 * This is an internal function that can be overridden to
 * change or augment the dictionary find behavior.
 *
 * @return pointer to value for key or NULL if there was no matching entry.
 *
 * @see Dict::Set Dict::Find
 */
template <class KEY, class VAL, class BASE>
	typename Dict<KEY, VAL, BASE>::Entry* Dict<KEY, VAL, BASE>::FindEntry(const KEY& key) const
{
	Entry*	entry;
    uint32	index = HashKey(key) & BASE::m_Mask;

    for (entry = (Entry*) BASE::m_Buckets[index];	// search all entries
		 entry != NULL;						// in our bucket
	     entry = (Entry*) entry->Next)
		if (CompareKeys(key, entry->Key))	// compare each key
			return entry;					// found a match!
    return NULL;
}


/*!
 * @fn VAL* Dict::Find(const KEY& key) const
 * @param key key for entry to find
 *
 * Finds the value of a dictionary entry given its key.
 * Only exact matches as defined by Dict::CompareKeys will return
 * a value.
 *
 * @return pointer to the value of the entry found, NULL if not found
 *
 * @see Dict::FindEntry Dict::Set
 */
template <class KEY, class VAL, class BASE>
	VAL* Dict<KEY, VAL, BASE>::Find(const KEY& key) const
{
	Entry* entry = FindEntry(key);
	if (entry)
		return &entry->Value;
    return NULL;
}

/*!
 * @fn Entry* Dict::MakeEntry(const KEY& key)
 * @param key key for entry to retrieve
 *
 * Returns a dictionary entry given its key.
 * Only exact matches as defined by Dict::CompareKeys>> will access
 * an existing entry. If no match is found in the dictionary,
 * the key is added to the dictionary and associated with an empty value.
 *
 * @return the dictionary entry found or created
 *
 * @see Dict::Find Dict::Set
 */
template <class KEY, class VAL, class BASE>
	typename Dict<KEY, VAL, BASE>::Entry* Dict<KEY, VAL, BASE>::MakeEntry(const KEY& key)
{
	Entry*	entry;
    uint32	index;

    index = HashKey(key) & BASE::m_Mask;
	if (entry = FindEntry(key))		// entry already there?
		return entry;				// just return it
/*
 * Entry not found. Add a new one to the bucket.
 */
	entry = new (BASE::GetEntryAllocator()) Entry(key);
	entry->Next = BASE::m_Buckets[index];
	BASE::m_Buckets[index] = entry;
	BASE::m_Size++;
/*
 * If the table has exceeded a decent size, rebuild it with many more buckets.
 */
    if (BASE::m_Size >= BASE::m_RebuildSize)
		BASE::Rebuild();
    return entry;
}


/*!
 * @fn Entry& Dict::operator[](const KEY& key)
 * @param key key for entry to retrieve
 *
 * Returns a dictionary entry given its key.
 * Only exact matches as defined by Dict::CompareKeys>> will access
 * an existing entry. If no match is found in the dictionary,
 * the key is added to the dictionary and associated with an empty value.
 *
 * @return the dictionary entry found or created
 *
 * @see Dict::Find Dict::Set
 */
template <class KEY, class VAL, class BASE>
	const typename Dict<KEY, VAL, BASE>::Entry& Dict<KEY, VAL, BASE>::operator[](const KEY& key) const
{
	return MakeEntry(key);
}

/*!
 * @fn Entry& Dict::operator[](const KEY& key)
 * @param key key for entry to retrieve
 *
 * Returns a dictionary entry given its key.
 * Only exact matches as defined by Dict::CompareKeys>> will access
 * an existing entry. If no match is found in the dictionary,
 * the key is added to the dictionary and associated with an empty value.
 *
 * @return the dictionary entry found or created
 *
 * @see Dict::Find Dict::Set
 */
template <class KEY, class VAL, class BASE>
	typename Dict<KEY, VAL, BASE>::Entry& Dict<KEY, VAL, BASE>::operator[](const KEY& key)
{
	typename Dict<KEY, VAL, BASE>::Entry * entry = MakeEntry(key);
	return *entry;
}
/*!
 * @fn VAL* Dict::Set(const KEY& key, const VAL& val)
 * @param key	key for the entry
 * @param val	new value for the entry
 *
 * Description:
 * Sets the value associated with a given key. If there is already
 * a dictionary entry with the input key, the value of that entry
 * is updated. Otherwise, a new entry is created with the given
 * key and value and added to the dictionary.
 *
 * @return reference to the value of the entry updated or added
 */
template <class KEY, class VAL, class BASE>
	VAL& Dict<KEY, VAL, BASE>::Set(const KEY& key, const VAL& val)
{
	typename Dict<KEY, VAL, BASE>::Entry * entry = MakeEntry(key);
	entry->Value = val;
	return entry->Value;
}

/*!
 * @fn void Dict::Merge(const Dict<KEY, VAL, BASE>& srcdict)
 * @param srcdict dictionary to merge with this one.
 *
 * Merges the entries of the source dictionary with this dictionary.
 * The source is left unaffected. All the entries in it are duplicated
 * within the destination.
 *
 * @see Dict::Set
 */
template <class KEY, class VAL, class BASE>
	void Dict<KEY, VAL, BASE>::Merge(const Dict<KEY, VAL, BASE>& srcdict)
{
	Iter	iter(srcdict);
	Entry*	entry;

	while (entry = iter.NextEntry())
		Set(entry->Key, entry->Value);
}

/*!
 * @fn uint32 Dict::HashKey(const KEY& key) const
 * @param key key to find hash index for
 *
 * Computes a one-word summary of an input key. You \b must supply
 * a version of this function for each different dictionary variant.
 *
 * @returns hash bucket index for input key
 *
 * @see Dict::CompareKeys BaseDict::HashStr BaseDict::HashInt
 */

/*!
 * @fn bool Dict::CompareKeys(const KEY& key1, const KEY& key2)
 * @param key1 key to find
 * @param key2 key from dictionary entry
 *
 * Compares a search key with a dictionary entry key. You \b must
 * supply a version of this function for each different dictionary variant.
 *
 * @returns \b true if keys are the same, else \b false
 */
template <class BASE> BaseDict<BASE>::BaseDict()
{
	m_Buckets = m_StaticBuckets;
	m_StaticBuckets[0] = m_StaticBuckets[1] = 0;
	m_StaticBuckets[2] = m_StaticBuckets[3] = 0;
	m_MaxSize = DICT_Small;
	m_Size = 0;
	m_RebuildSize = DICT_Small * DICT_RebuildSize;
	m_Shift = 28;
	m_Mask = 3;
	m_EntryAlloc = NULL;
}

/*!
 * Hashes integer key and returns bucket index.
 * Takes a preliminary integer hash value and produces an index
 * into a hash tables bucket list.  The idea is to make it so
 * that preliminary values that are arbitrarily similar will
 * end up in different buckets.  The hash function was taken
 * from a random-number generator.
 *
 * @see HashStr
 */
template <class BASE>
	uint32	BaseDict<BASE>::HashInt(intptr k) const
{
	return (uint32) (k * 1103515245) >> m_Shift;
}

/*!
 * @fn uint32 BaseDict::HashStr(const TCHAR*) const
 *
 * Computes a one-word summary of a text string, which can be
 * used to generate a hash index. The algorithm multiplies
 * the accumulated hash key by 9 and adds each new character.
 * @li	Multiplying by 10 is perfect for keys that are decimal strings,
 *		and multiplying by 9 is just about as good.
 * @li	Times-9 is (shift-left-3) plus (old).  This means that each
 *		character's bits hang around in the low-order bits of the
 *		hash value for ever, plus they spread fairly rapidly up to
 *		the high-order bits to fill out the hash value.  This seems
 *		works well both for decimal and non-decimal strings.
 *
 * @returns one-word summary of the information in string
 *
 * @see Dict::CompareKeys Dict::HashKey BaseDict::HashInt
 */
template <class BASE>
	uint32 BaseDict<BASE>::HashStr(const TCHAR* s) const
{
	uint32	c, result = 0;

    while (c = *s++)
	{
		result += (result << 3);
		if (isalpha(c))
			result += tolower(c);
		else
			result += c;
	}
    return result;
}

/*!
 * @fn void BaseDict::Rebuild()
 *
 * This procedure is invoked when the ratio of entries to hash
 * buckets becomes too large.  It creates a larger bucket array and
 * moves all of the entries into the new buckets. A side effect is that
 * memory gets reallocated and entries get re-hashed to new	buckets.
 */
template <class BASE> void BaseDict<BASE>::Rebuild()
{
    int		oldSize, count, index;
    List**	oldBuckets;
    List**	oldChainPtr, **newChainPtr;
    List*	entry;

    oldSize = m_MaxSize;
    oldBuckets = m_Buckets;
/*
 * Allocate and initialize the new bucket array, and set up
 * hashing constants for new array size.
 */
    m_MaxSize *= 4;
    m_Buckets = (List**) malloc(m_MaxSize * sizeof(List*));
    for (count = m_MaxSize, newChainPtr = m_Buckets;
	     count > 0;
		 count--, newChainPtr++)
		*newChainPtr = NULL;
	m_RebuildSize *= 4;
    m_Shift -= 2;
    m_Mask = (m_Mask << 2) + 3;
/*
 * Rehash all of the existing entries into the new bucket array.
 */
    for (oldChainPtr = oldBuckets;
		 oldSize > 0;
		 oldSize--, oldChainPtr++)
	{
		for (entry = *oldChainPtr;
			 entry != NULL;
			 entry = *oldChainPtr)
		{
			*oldChainPtr = entry->Next;
			index = HashEntry(entry) & m_Mask;  
		    entry->Next = m_Buckets[index];
		    m_Buckets[index] = entry;
		}
	}
/*
 * Free up the old bucket array, if it was dynamically allocated.
 */
    if (oldBuckets != m_StaticBuckets)
		free(oldBuckets);
}

/*!
 * @fn BaseDict::Empty
 *
 * Removes all the dictionary entries, leaving the dictionary entry.
 * Storage used by the entries, keys and values is reclaimed.
 */
template <class BASE> void BaseDict<BASE>::Empty()
{
    List*	entry, *next;
    int		i;

    for (i = 0; i < m_MaxSize; i++)		// free all dict entries
	{
		next = m_Buckets[i];			// for each bucket
		while (entry = next)			// for each entry
		{
	    	next = entry->Next;			// unlink entry
			entry->Next = NULL;
			DeleteEntry(entry);
		}
	}
    if (m_Buckets != m_StaticBuckets)	// bucket array allocated?
		free(m_Buckets);				// free bucket array

	m_Buckets = m_StaticBuckets;
    m_StaticBuckets[0] = m_StaticBuckets[1] = 0;
    m_StaticBuckets[2] = m_StaticBuckets[3] = 0;
    m_MaxSize = DICT_Small;
    m_Size = 0;
    m_RebuildSize = DICT_Small * DICT_RebuildSize;
    m_Shift = 28;
    m_Mask = 3;
}

} // end Core