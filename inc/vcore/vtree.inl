namespace Core {

template <class BASE> Tree<BASE>::Tree()
  :	BASE(), m_Parent(NULL), m_First(NULL), m_Next(NULL) { }

template <class BASE> Tree<BASE>::~Tree()
{ Empty(); }

/*!
 * @fn void Tree::Empty()
 *
 * Unlinks all child nodes from this parent.
 */
template <class BASE> void Tree<BASE>::Empty()
{
	Tree<BASE>* pred = m_First;

	while (pred)
	{
		Tree<BASE>* c = pred->m_Next;
		pred->m_Parent = NULL;
		pred->m_Next = NULL;
		pred->m_First = NULL;
		pred = c;
	}
	m_First = NULL;
}

template <class BASE> int Tree<BASE>::GetSize() const
{
	int size = 0;
	const Tree<BASE>* c = m_First;

	while (c)
	{
		++size;
		c = c->m_Next;
	}
	return size;
}

/*!
 * @fn Tree<BASE>* Tree::GetAt(int n)
 * @param	n	zero-based index of a child of this tree.
 * @return	pointer to the \b nth child, NULL if there aren't enough children.
 */
template <class BASE> Tree<BASE>* Tree<BASE>::GetAt(int n)
	{ return (Tree<BASE>*) ((const Tree<BASE>*) this)->GetAt(n); }

template <class BASE> const Tree<BASE>* Tree<BASE>::GetAt(int n) const
{
	const Tree<BASE>* prev = m_First;

	if (n < 0)
		return NULL;
	while (prev && (--n >= 0))
		prev = prev->m_Next;
	return prev;
}

template <class BASE> Tree<BASE>* Tree<BASE>::Last()
	{ return (Tree<BASE>*) ((const Tree<BASE>*) this)->Last(); }
	
template <class BASE> const Tree<BASE>* Tree<BASE>::Last() const
{
	const Tree<BASE>* prev = m_First;

	while (prev)
	{
		if (prev->m_Next == NULL)
			return prev;
		prev = prev->m_Next;
	}
	return NULL;
}

/*!
 * @fn Tree<BASE>* Tree::Prev()
 * If this node has no parent or it is the first child, NULL is returned.
 */
template <class BASE> Tree<BASE>*  Tree<BASE>::Prev()
{	return (Tree<BASE>*) ((const Tree<BASE>*) this)->Prev(); }

template <class BASE> const Tree<BASE>*  Tree<BASE>::Prev() const
{
	// Need to find previous item using parent node
	// If parent's firstChild points at this node, previous is NULL
	if (m_Parent && m_Parent->m_First != this)
	{
		for (const Tree<BASE>* prev = m_Parent->m_First; prev; prev = prev->m_Next)
			if (prev->m_Next == this)
				return prev;
		// Must find this node, otherwise we have invalid linkages
		assert(false);
	}

	return NULL;
}

/*!
 * @fn bool Tree::Remove(bool free)
 * @param free	If \b true, free the node after unlinking. The default is to free the node.
 *
 * This node and all of its children are treated as a unit and unlinked from the parent.
 * The default behavior is to unlink the node but not delete it.
 * The caller may request to delete the node by passing \b true.
 * The node is deleted using the system \b delete call.
 *
 * @return	This node if it was removed or \b NULL on error.
 *
 * @see Tree::Append Tree::Empty RefTree::Remove
 */
template <class BASE> bool Tree<BASE>::Remove(bool free)
{
	Tree<BASE>* par = m_Parent;
    Tree<BASE>* pred;

    if (par == NULL)
		return false;
    pred = m_Parent->m_First;
    if (pred == NULL)
		VX_ERROR(("VTree_Remove: ERROR hierarchy linkage broken"), false);
    if (pred == this)						// remove the first one?
	    par->m_First = m_Next;
	else
	{
		pred = Prev();						// find its predecessor
		VX_ASSERT(pred != NULL);
		pred->m_Next = m_Next;				// unlink from group
	}
	m_Next = NULL;							// mark as unlinked
	m_Parent = NULL;
	if (free)								// free the node?
		delete this;
	return true;
}


/*!
 * @fn bool Tree::Append(Tree<BASE>* child)
 * @param child	tree to add as last child.
 *
 *	The input node is put at the end of the list of children
 *	for this node, which becomes its parent.
 *	
 * @return If successful, a pointer to the child that was added.
 *	A NULL return value indicates the child could not be added
 *	because it was already in another hierarchy.
 *
 * @see Tree::PutAfter Tree::PutBefore RefTree::Append
 */
template <class BASE> bool Tree<BASE>::Append(Tree<BASE>* child)
{
	VX_ASSERT(child != this);
	if (child->m_Parent != NULL)
		VX_ERROR(("VTree_Append: ERROR input node already in a hierarchy\n"), false);
	VX_ASSERT(child->m_Next == NULL);
    if (m_First == NULL)				/* no children yet? */
	    m_First = child;				/* put it at the front */
	else
	{
		Tree<BASE>*	last = m_First;
		while (last->m_Next)			/* find last element */
			last = last->m_Next;
		last->m_Next = child;			/* insert child in this group */
	}
	child->m_Parent = this;				/* this group is the parent */
    return true;
}

/*!
 * @fn bool Tree::PutFirst(Tree<BASE>* child)
 * @param child	tree to add as first child.
 *
 *	The input node is put at the start of the list of children
 *	for this node, which becomes its parent.
 *	
 * @return If successful, a pointer to the child that was added.
 *	A NULL return value indicates the child could not be added
 *	because it was already in another hierarchy.
 *
 * @see Tree::Append Tree::PutBefore RefTree::PutFirst
 */
template <class BASE> bool Tree<BASE>::PutFirst(Tree<BASE>* child)
{
	VX_ASSERT(child != this);
	if (child->m_Parent != NULL)
		VX_ERROR(("VTree_PutFirst: ERROR input node already in a hierarchy\n"), false);
	VX_ASSERT(child->m_Next == NULL);
	child->m_Parent = this;
	child->m_Next = m_First;
	m_First = child;
    return true;
}

/*!
 * @fn bool Tree::PutAfter(Tree<BASE>* afterme)
 * @param afterme	sibling to insert this node after.
 *
 *	This node and the input node will share the same parent.
 *	Upon return, the input nodewill precede this node in the hierarchy.
 *	
 * @return \b true if node successfully linked, else \b false
 *
 * @see  RefObj Tree::Append Tree::PutBefore
 */
template <class BASE> bool Tree<BASE>::PutAfter(Tree<BASE>* afterme)
{
	if ((m_Parent != NULL) || (afterme->m_Parent == NULL))
		VX_ERROR(("VTree_PutAfter: ERROR this node already in a hierarchy\n"), false);
	VX_ASSERT(m_Next == NULL);
    m_Next = afterme->m_Next;
    m_Parent = afterme->m_Parent;
    afterme->m_Next = this;
	return true;
}

/*!
 * @fn bool Tree::PutBefore(Tree<BASE>* before)
 * @param before	sibling to insert this node before.
 *
 *	This node and the input node will share the same parent.
 *	Upon return, the input node will come after this node in the hierarchy.
 *
 * @return \b true if node successfully linked, else \b false
 *
 * @see Tree::Append Tree::PutAfter RefTree::PutBefore
 */
template <class BASE> bool Tree<BASE>::PutBefore(Tree<BASE>* before)
{
	if ((m_Parent != NULL) ||
		(before->m_Parent == NULL))
		VX_ERROR(("VTree_PutBefore: ERROR input node already in a hierarchy"), false);

	Tree<BASE>* pred = before->m_Parent->m_First;
	if (pred == NULL)
		return false;
    if (pred == before)						/* desired predecessor is first thing in list? */
	    before->m_Parent->m_First = this;	/* link it in */
	else
	{
		while (pred->m_Next != before)		/* find predecessor */
		{
			pred = pred->m_Next;
			if (pred == NULL)				/* end of list? */
				VX_ERROR(("VTree_PutBefore: ERROR broken hierarchy linkage detected"), false);
		}
		pred->m_Next = this;				/* after "pred" */
	}
	m_Next = before;						/* link in at the front */
    m_Parent = before->m_Parent;			/* make this group its parent */
	return true;
}

/*!
 * @fn bool Tree::Replace(Tree<BASE>* src)
 * @param src	node to put in place of this node.
 *
 *	Takes this node out of the hierarchy puts the source
 *	node in its place. Both nodes are treated as a subtrees
 *	and their child linkage is not disturbed by the replacement.
 *	This node is  not freed, just removed from the hierarchy.
 *
 * @return The tree which was removed or \b NULL on error
 *
 * @see Tree::Append Tree::Remove
 */
template <class BASE> bool Tree<BASE>::Replace(Tree<BASE>* src)
{
	if (src == this)
		return false;
	if (!src->PutAfter(this))			// put input node after this node
		return false;
	Remove(false);						// remove this node, don't free it
    return true;
}

template <class BASE> inline
RefTree<BASE>::RefTree() : Tree<BASE>() { }

/*!
 * @fn bool RefTree::Remove(bool free)
 * @param free	If \b true, free the node after unlinking.
 *
 * This node and all of its children are treated as a unit and unlinked from the parent.
 * The default behavior is to dereference the node but not delete it.
 * The caller may request to delete the node by passing \b true.
 *
 * If a free is not requested, the reference count on the node
 * is decremented but no resources are freed (the default case for
 * reference counted nodes). If the caller requests the node to be freed,
 * its reference count is decremented and it is garbage collected if there
 * are no outstanding references left.
 *
 * @return	This node if it was removed or \b NULL on error.
 *
 * @see RefObj RefTree::Append Tree::Remove
 */
template <class BASE> bool RefTree<BASE>::Remove(bool free)
{
	if (!Tree<BASE>::Remove(false))
		return false;
	if (free)
		Tree<BASE>::Delete();				// delete self
	else
	{
		vint32* vptr = &(Tree<BASE>::m_refCount);
		Core::InterlockDec(vptr);// restore use count
	}
	return true;
}

/*!
 * @fn bool RefTree::Append(Tree<BASE>* child)
 * @param child	tree to add as last child.
 *
 *	The input node is put at the end of the list of children
 *	for this node, which becomes its parent.
 *	Putting a child in a tree adds another reference to the child
 *	(increments the use count).
 *	
 * @return If successful, a pointer to the child that was added.
 *	A NULL return value indicates the child could not be added
 *	because it was already in another hierarchy.
 *
 * @see RefObj RefTree::PutAfter RefTree::PutBefore Tree::Append
 */
template <class BASE> bool RefTree<BASE>::Append(Tree<BASE>* child)
{
	if (Tree<BASE>::Append(child))
	{
		child->IncUse();
		return true;
	}
	return false;
}

/*!
 * @fn bool RefTree::PutFirst(Tree<BASE>* child)
 * @param child	tree to add as first child.
 *
 *	The input node is put at the start of the list of children
 *	for this node, which becomes its parent.
 *	Putting a child in a tree adds another reference to the child
 *	(increments the use count).
 *	
 * @return If successful, a pointer to the child that was added.
 *	A NULL return value indicates the child could not be added
 *	because it was already in another hierarchy.
 *
 * @see Tree::Append Tree::PutBefore RefTree::PutFirst
 */
template <class BASE> bool RefTree<BASE>::PutFirst(Tree<BASE>* child)
{
	if (Tree<BASE>::PutFirst(child))
	{
		child->IncUse();
		return true;
	}
	return false;
}

/*!
 * @fn bool RefTree::PutAfter(Tree<BASE>* after)
 * @param after	sibling to insert this node after.
 *
 *	This node and the input node will share the same parent.
 *	Upon return, the input node will precede this node in the hierarchy.
 *	Putting a child in a tree adds another reference to the child
 *	(increments the use count).
 *
 * @return \b true if node successfully linked, else \b false
 *
 * @see  RefObj RefTree::Append RefTree::PutBefore Tree::PutAfter
 */
template <class BASE> bool RefTree<BASE>::PutAfter(Tree<BASE>* after)
{
	if (Tree<BASE>::PutAfter(after))
	{
		Tree<BASE>::IncUse();
		return true;
	}
	return false;
}

/*!
 * @fn bool RefTree::PutBefore(Tree<BASE>* before)
 * @param before	sibling to insert this node before.
 *
 *	This node and the input node will share the same parent.
 *	Upon return, the input node will come after this node in the hierarchy.
 *	Putting a child in a tree adds another reference to the child
 *	(increments the use count).
 *
 * @return \b true if node successfully linked, else \b false
 *
 * @see VRef RefObj RefTree::Append RefTree::PutAfter
 */
template <class BASE> bool RefTree<BASE>::PutBefore(Tree<BASE>* before)
{
	if (Tree<BASE>::PutBefore(before))
	{
		Tree<BASE>::IncUse();
		return true;
	}
	return false;
}

/*!
 * @fn bool RefTree::Replace(Tree<BASE>* src)
 * @param src	node to put in place of this node.
 *
 *	Takes this node out of the hierarchy puts the source
 *	node in its place. Both nodes are treated as a subtrees
 *	and their child linkage is not disturbed by the replacement.
 *	This node is dereferenced when removed but not deleted (its use count is decremented).
 *	If its only use was in the hierarchy, it now has the same status
 *	as if it were returned by \b new. The caller now has
 *	responsibility to garbage collect this tree.
 *
 * @return The tree which was removed or \b NULL on error
 *
 * @see RefTree::Append RefTree::Remove Tree::Replace
 */
template <class BASE> bool RefTree<BASE>::Replace(Tree<BASE>* src)
{
	return Tree<BASE>::Replace(src);
}

/*!
 * @fn void RefTree::Empty()
 *
 * Unlinks all child nodes from this parent and garbage collects them.
 * The reference count of each child is decremented and the
 * child is garbage collected if necessary.
 *
 * @see RefObj RefTree::Remove Tree::Empty
 */
template <class BASE> void RefTree<BASE>::Empty()
{
	RefTree<BASE>* pred = (RefTree<BASE>*) Tree<BASE>::First();

	while (pred)
	{
		RefTree<BASE>* c = (RefTree<BASE>*) pred->m_Next;
		pred->m_Parent = NULL;
		pred->m_Next = NULL;
		pred->Delete();	
		pred = c;
	}
	Tree<BASE>::m_First = NULL;
}

template <class BASE> RefTree<BASE>::~RefTree()
{
	Empty();
}

} // end Core