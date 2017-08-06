#ifndef _melist_loaded
#define _melist_loaded

/*!
 * @class meList
 * @brief A list is used to link objects together in a chain.
 *
 * Each object in a list can reference one other object.
 * An object may only belong to one list - it cannot have
 * multiple followers.
 */
class meList
{
public:
    meList();					//!< Make an empty list.
    virtual ~meList();				//!< Delete our followers.
    meList*	  last();			//!< Return last node in this list.
    bool	  isLast() const;		//!< Returns \b true if this node is last. 
    bool	  contains(const meList*) const;
    int		  getSize() const;		//!< Returns number of of objects in this list.
    meList*	  getAt(int n);			//!< Retrieves a list element based on index.
    meList*	  operator[](int n);		//!< Retrieves a list element based on index.
    bool	  append(meList* last);		//!< Puts this node at the end.
    bool	  putAfter(meList* after);	//!< Puts this node after the given node.
    meList*	  remove(meList* node);		//!< Removes this node from the list.
    void	  empty();			//!< Unlinks and deletes all followers.
    meList*  	  pred(meList*);		//!< Returns the predecessor of the input node.

//  Data members
    meList*	Next;				//!< Pointer to next list element.
};

inline meList::meList()
{   Next = NULL; }

inline bool meList::isLast() const
{   return Next == NULL; }

inline meList* meList::operator[](int n)
{ return getAt(n); }

#endif
