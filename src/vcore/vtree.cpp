#include "vcore/vcore.h"

namespace Vixen {
namespace Core {

List::~List() { }

void List::Empty()
{
	List* next = Next;
	Next = NULL;
	if (next)
		next->Empty();
}

int List::GetSize() const
{
	int size = 0;
	const List* c = this;

	while (c)
	{
		++size;
		c = c->Next;
	}
	return size;
}

/*!
 * @fn List* List::GetAt(int n)
 * @param	n	zero-based index of follower to get.
 * @return	pointer to the \b nth follower, NULL if there aren't enough followers.
 */
List* List::GetAt(int n)
	{ return (List*) ((const List*) this)->GetAt(n); }

const List* List::GetAt(int n) const
{
	const List* prev = this;

	if (n < 0)
		return NULL;
	while (prev && (n-- >= 0))
		prev = prev->Next;
	return prev;
}

List* List::Last()
{
	if (Next)
		return Next->Last();
	return this;
}
	
const List* List::Last() const
{
	if (Next)
		return Next->Last();
	return this;
}

/*!
 * @fn List* List::Pred(List* node)
 * @param node	Node to find predecessor for.
 *
 * If the given node is found among the followers of this list,
 * it's predecessor is returned.
 *
 * @return	Predecessor of the input node. NULL is returned if the input node
 * is not a follower of this node or if they are the same.
 *
 * @see List::Append List::Empty
 */
List* List::Pred(List* node)
{
	List* pred = this;

	while (pred->Next)
	{
		if (pred->Next == node)
			return pred;
		pred = pred->Next;
	}
	return NULL;
}

bool List::Contains(const List* node) const
{
	const List* pred = this;

	while (pred->Next)
		if (pred->Next == node)
			return true;
	return false;
}

/*!
 * @fn List* List::Remove(List* node)
 * @param node	Node to remove from this list.
 *
 * If the given node is found among the followers of this list, it is removed.
 *
 * @return	node which was removed or \b NULL on error.
 *
 * @see List::Append List::Empty
 */
List* List::Remove(List* node)
{
	List* pred = Pred(node);

	if (pred)
	{
		pred->Next = node->Next;
		node->Next = NULL;
		return node;
	}
	return NULL;
}


/*!
 * @fn bool List::Append(List* child)
 * @param child	node to add at the end of the list.
 *
 * @return \b true if successful, \b false if node could not be added
 *	because it was already in another list.
 *
 * @see List::PutAfter
 */
bool List::Append(List* child)
{
	if (child->Next != NULL)
		return false;
	Last()->Next = child;
    return true;
}

/*!
 * @fn bool List::PutAfter(List* follow)
 * @param follow	node to put this node after.
 *
 * Upon return, the input node will precede this node in the list.
 *
 * @return \b true if node successfully linked, else \b false
 *
 * @see  List::Append
 */
bool List::PutAfter(List* follow)
{
	Next = follow->Next;
	follow->Next = this;
	return true;
}


}	// end Core
}	// end Vixen