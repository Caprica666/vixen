#pragma once

/*!
 * @fn bool NameIter::NextWild()
 *
 * Retrieves the next dictionary entry whose name matches the search string.
 *
 * @return pointer to next matching entry or NULL if no more
 *
 * @see NameIter::CompareWild NameDict::FindWild
 */
template <class ITEM> typename Dictionary<NameProp, ObjRef>::Entry* NameIter<ITEM>::NextWild()
{
	Dictionary<NameProp, ObjRef>::Entry *	entry;

	while (entry = Dictionary<NameProp, ObjRef>::Iter::NextEntry())
		if ((m_SearchKey == NULL) || CompareWild(entry->Key))
			return entry;
	return NULL;
}

/*!
 * @fn bool NameIter::CompareWild(const NameProp& kdict) const
 * @param kdict dictionary key
 *
 * Compares a dictionary key against a search key. This function supports
 * the concept of a wildcard match between string keys. The wildcard
 * "*" will match any sequence of characters in a dictionary key.
 * Putting an asterisk at the beginning of the search string will
 * match if the dict entry ends with that string. Similarly, putting
 * an asterisk at the end of the new key will match if the dict key
 * starts with that string. You can put an asterisk at both ends
 * to match if the entry contains the string.
 *
 * @return \b true if the search key matches the dictionary key, else \b false
 *
 * @see NameDict::Find NameDict::FindWild
 */
template <class ITEM> bool NameIter<ITEM>::CompareWild(const NameProp& kdict) const
{
	const TCHAR* a = m_SearchKey;
	const TCHAR* b = kdict;
	size_t			n = STRLEN(a) - 1;
	bool			match_to_end = (a[n] != '*');

	if (*a == '*')					// starts with wildcard
	{
		++a;
		while (*b)					// return if matches rightmost
		{
			if (*b++ == *a)
			{
				if (match_to_end)	// need complete match?
				{
					if (STRCASECMP(b, a+1) == 0)
						return true;
				}
				else				// doesn't need to match to end of both
				{
					if (STRNCASECMP(b, a+1, n - 2) == 0)
						return true;
				}
			}
		}

		return false;				// no match found
	}
	if (match_to_end)				// match until end of both strings?
		return STRCASECMP(b, a) == 0;
	else							// no, just until end of first one
		return STRNCASECMP(b, a, n - 1) == 0;
}


/**
 * @fn ITEM* NameDict::FindWild(const char *name) const
 *
 * Performs a wildcard search for a dictionary entry based on its key.
 * The wildcard "*" will match any sequence of characters in the key.
 * Putting an asterisk at the beginning of the name search string will
 * match the first entry which ends with that string. Similarly, putting
 * an asterisk at the end of the name string will match the first entry
 * which starts with that string. You can put an asterisk at both ends
 * to match the entries which contain the string.
 *
 * Hash order is indeterminate so this routine is not generally useful
 * if you expect more than one entry to match. For multiple entry
 * wildcard searches, the \b FindAll function is useful.
 *
 * @see NameDict::Find
 */
template <class ITEM> ITEM* NameDict<ITEM>::FindWild(const TCHAR* name) const
{
	Dictionary<NameProp, ITEM>* cheat = (Dictionary<NameProp, ITEM>*) this;
	NameIter<ITEM>	iter(cheat, name);

	typename Dictionary<NameProp, ITEM>::Entry *	e = iter.NextWild();
	if (e)
		return &(e->Value);
	else
		return NULL;
}

/**
 * @fn ITEM* NameDict::Find(const char *name) const
 *
 * Finds the value associated with the given string.
 * This is the fastest way to access values but will only
 * perform an exact match on the string name.
 *
 * @see NameDict::FindWild Dictionary::Find
 */
template <class ITEM> inline ITEM* NameDict<ITEM>::Find(const TCHAR* name) const
{
	NameProp	np(name);
	return Dictionary<NameProp, ITEM>::Find(np);
}

/**
 * @fn ObjArray<ITEM> NameDict::FindAll(const char *name) const
 *
 * Performs a wildcard search for a dictionary entry based on its key.
 * The wildcard "*" will match any sequence of characters in the key.
 * Putting an asterisk at the beginning of the name search string will
 * match the first entry which ends with that string. Similarly, putting
 * an asterisk at the end of the name string will match the first
 * which starts with that string. You can put an asterisk at both ends
 * to match the entries which contain the string.
 *
 * Hash order is indeterminate so this routine is not generally useful
 * if you expect more than one entry to match. For multiple entry
 * wildcard searches, the \b FindAll function is useful.
 *
 * @see NameDict::Find
 */
template <class ITEM> Array<ITEM>* NameDict<ITEM>::FindAll(const TCHAR* name) const
{
	NameIter<ITEM> iter((Dictionary<NameProp, ITEM>*) this, name);
	typename Dictionary<NameProp, ITEM>::Entry * e;
	Array<ITEM>* arr = new Array<ITEM>();

	while (e = ((typename Dictionary<NameProp, ITEM>::Entry * ) iter.NextWild() ))
		arr->Append(e->Value);
	if (arr->GetSize() == 0)
	{
		arr->Delete();
		return NULL;
	}
	return arr;
}
