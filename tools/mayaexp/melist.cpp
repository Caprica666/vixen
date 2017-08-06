#ifdef WIN32
#include "pfb_all.h"

#else
#include <stddef.h>
#include "melist.h"
#endif

meList::~meList() { }

void meList::empty()
{
    meList* next = Next;
    Next = NULL;
    if (next)
        next->empty();
}

int meList::getSize() const
{
    int size = 0;
    const meList* c = this;

    while (c)
    {
        ++size;
        c = c->Next;
    }
    return size;
}

/*!
 * @fn meList* meList::getAt(int n)
 * @param	n	zero-based index of follower to get.
 * @return	pointer to the \b nth follower, NULL if there aren't enough followers.
 */
meList* meList::getAt(int n)
{
    meList* prev = this;

    if (n < 0)
        return NULL;
    while (prev && (n-- >= 0))
        prev = prev->Next;
    return prev;
}

meList* meList::last()
{
    if (Next)
        return Next->last();
    return this;
}

/*!
 * @fn meList* meList::pred(meList* node)
 * @param node	Node to find predecessor for.
 *
 * If the given node is found among the followers of this list,
 * it's predecessor is returned.
 *
 * @return	Predecessor of the input node. NULL is returned if the input node
 * is not a follower of this node or if they are the same.
 *
 * @see meList::append meList::empty
 */
meList* meList::pred(meList* node)
{
    meList* pred = this;

    while (pred->Next)
    {
        if (pred->Next == node)
    	return pred;
        pred = pred->Next;
    }
    return NULL;
}

bool meList::contains(const meList* node) const
{
    const meList* pred = this;

    while (pred->Next)
        if (pred->Next == node)
    	return true;
    return false;
}

/*!
 * @fn meList* meList::remove(meList* node)
 * @param node	Node to remove from this list.
 *
 * If the given node is found among the followers of this list, it is removed.
 *
 * @return	node which was removed or \b NULL on error.
 *
 * @see meList::append meList::empty
 */
meList* meList::remove(meList* node)
{
    meList* prev = pred(node);

    if (prev)
    {
        prev->Next = node->Next;
        node->Next = NULL;
        return node;
    }
    return NULL;
}


/*!
 * @fn bool meList::append(meList* node)
 * @param child	node to add at the end of the list.
 *
 * @return \b true if successful, \b false if node could not be added
 *	because it was already in another list.
 *
 * @see meList::putAfter
 */
bool meList::append(meList* node)
{
    if (node->Next != NULL)
        return false;
    last()->Next = node;
    return true;
}

/*!
 * @fn bool meList::putAfter(meList* pred)
 * @param pred	node to put this node after.
 *
 * Upon return, the input node will precede this node in the list.
 *
 * @return \b true if node successfully linked, else \b false
 *
 * @see  meList::append
 */
bool meList::putAfter(meList* pred)
{
    Next = pred->Next;
    pred->Next = this;
    return true;
}

