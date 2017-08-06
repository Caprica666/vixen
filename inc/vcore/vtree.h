#pragma once

/*!
 * @file vtree.h
 * @brief Hierarchy base classes and iterator.
 *
 * @author Nola Donato
 *
 * @ingroup vcore
 * @see varray.h vdict.h
 */


namespace Core {

/*!
 * @brief A list is used to link objects together in a chain.
 *
 * Each object in a list can reference one other object.
 * An object may only belong to one list - it cannot have
 * multiple followers.
 *
 * Although no public access is given to the group linkage pointers,
 * you can use the VTreeIter template class to iterate over the group 
 * in a variety of ways.
 *
 * @ingroup vcore
 * @see BaseObj Tree Array Allocator
 */
class List : public BaseObj
{
public:
	//! Make an empty list.
	List() : BaseObj() { Next = NULL; }
	//! Delete our followers.
	virtual ~List();

	//! Return last node in this list.
	List*			Last();
	const List*		Last() const;
	bool			IsLast() const		{ return Next == NULL; }
	//! Returns \b true if the given node follows this node.
	bool			Contains(const List*) const;
	//! Returns number of objects which follow this one (number of objects in list).
	int				GetSize() const;
	//! Retrieves a node from a list based on its position relative to this node.
	List*			GetAt(int n);
	const List*		GetAt(int n) const;
	//! Retrieves a node from a list based on its position relative to this node.
	List*			operator[](int n)		{ return GetAt(n); }
	const List*		operator[](int n) const	{ return GetAt(n); }

	//! Puts this node after the last node in the list.
	bool			Append(List* child);
	//! Puts this node after the list node.
	bool			PutAfter(List* follow);
	//! Removes this node from this list list.
	List*			Remove(List* node);
	//! Unlinks and deletes all the followers of this node.
	void			Empty();
	//! Returns the predecessor of the input node.
	List*			Pred(List*);
	const List*		Pred(List*) const;


//	Data members
	List*	Next;
};


/*!
 * @class Tree
 * @param BASE	base class to inherit from, must be subclass of BaseObj
 *
 * @brief A tree is used as the building block for hierarchies.
 *
 * Each node in a tree can contain, or be a parent to,
 * any number of children. Within a hierarchy, any level of nesting is permitted
 * but circular references are not allowed. A tree node can only be a child in
 * one hierarchy - it cannot have multiple parents.
 *
 * Although no public access is given to the tree linkage pointers,
 * you can use the local \b Iter class to iterate over the tree 
 * in a variety of ways.
 *
 * @ingroup vcore
 * @see BaseObj RefObj Tree::Iter RefTree
 */
template <class BASE> class Tree : public BASE
{
public:
	/*!
	 * @brief Options for traversing a tree, determines order in which nodes are returned.
	 */
	enum 
	{ 
		DEPTH_FIRST = 1,	//!< depth first traversal, returning children before parents
		PARENTS_REVERSE,	//!< traverses from root (top level parent) to this node's parent
		CHILDREN,			//!< returns only first level children, first to last
		CHILDREN_REVERSE,	//!< returns only first level children, last to first
		SEARCH_MASK = 7,
	};

	/*!
	 * @brief Iterator used to traverse hierarchies made of Tree subclasses.
	 *
	 * The constructor initializes the iterator to traverse a particular tree.
	 * There are several options for how to traverse the hierarchy
	 * which determine the order in which nodes are returned.
	 * Each time the Iter::Next function is called, a pointer to
	 * a different node in the hierarchy is returned. When a NULL value
	 * is returned, all the nodes have been visited.
	 *
	 * @see Tree
	 */
public:
	class Iter
	{
	protected:
		const Tree<BASE>*	m_Group;
		Tree<BASE>*		(Iter::*m_Func)();
		Tree<BASE>*		m_Last;
		Tree<BASE>*		m_Cur;

	public:
	/*!
	 * Makes an iterator to walk a hierarchy with a specified iteration method.
	 * @param root	Root of hierarchy to iterate.
	 * @param method	Hierarchy iteration method.
	 */
		Iter(Tree<BASE>* root, int method = DEPTH_FIRST)
		{ m_Group = (const Tree<BASE>*) root; m_Cur = NULL; m_Last = NULL; Reset(method); }

		Iter(const Tree<BASE>* root, int method = DEPTH_FIRST)
		{ m_Group = root; m_Cur = NULL; m_Last = NULL; Reset(method); }
	/*!
	 * Returns the next node in the hierarchy iteration, \b NULL when iteration complete.
	 */
		Tree<BASE>*	Next()		{ return (this->*m_Func)(); }
		Tree<BASE>*	Current()	{ return m_Cur; }
	/*!
	 * Resets iterator to walk the hierarchy again using the given iteration method.
	 * @param method	Hierarchy iteration method.
	 */
		void	Reset(int method = DEPTH_FIRST)
		{
			m_Cur = (Tree<BASE>*) m_Group;
			switch (method & SEARCH_MASK)
			{
				case PARENTS_REVERSE:			/* parents only */
				m_Func = &Tree<BASE>::Iter::ParentsReverse;
				break;

				case CHILDREN:					/* top level children only */
				m_Func = &Tree<BASE>::Iter::KidsOnly;
				m_Cur = (Tree<BASE>*) m_Group->First();
				break;

				case CHILDREN_REVERSE:			/* top level children reversed */
				m_Func = &Tree<BASE>::Iter::KidsOnlyReverse;
				m_Cur = (Tree<BASE>*) m_Group->Last();
				break;

				default:							/* depth-first search */
				m_Func = &Tree<BASE>::Iter::DepthFirst;
				m_Cur = (Tree<BASE>*) m_Group;		/* start at top group */
				while (m_Cur->First())				/* find lowest level child */
					m_Cur = m_Cur->First();
			}
		}

	protected:
		/****
		 *
		 * traversal for TREE_KidsOnly option
		 * Return the top-level children of the group only.
		 * Do not descend to lower-level children.
		 *
		 * notes:
		 * On entry, m_Cur points to the first child of the group
		 * (or NULL if it has no children)
		 *
		 ****/
		Tree<BASE>* KidsOnly()
		{
			Tree<BASE>* cur = m_Cur;

			if (cur)
				m_Cur = cur->m_Next;
			return cur;
		}

		/****
		 *
		 * traversal for TREE_KidsOnlyReverse option
		 * Return the top-level children of the group only in reverse order.
		 * Do not descend to lower-level children.
		 *
		 * notes:
		 * On entry, m_Cur points to the last child of the group
		 * (or NULL if it has no children)
		 *
		 ****/
		Tree<BASE>* KidsOnlyReverse()
		{
			Tree<BASE>* cur = m_Cur;

			if (cur)
				m_Cur = m_Cur->Prev();
			return cur;
		}

		/****
		 *
		 * traversal for TREE_ParentsReverse option
		 * Return parent nodes starting at the tree root to the target node.
		 * Iteration ends once the target node is reached (target node *not* returned).
		 *
		 * notes:
		 * On entry, m_Cur points to the tree node
		 *
		 ****/
		Tree<BASE>* ParentsReverse()
		{
			Tree<BASE>* cur = m_Cur;

			// End of iteration?
			if (cur == NULL) return NULL;

			// Found the last parent?
			if (m_Last->Parent() == m_Cur)
				m_Cur = NULL;

			// Walk the parent/child link to find next node.
			else
			{
				// Start at the target and work up the hierarchy until next lowest
				// level beyond m_current is reached.
				m_Cur = m_Last;
				while (m_Cur->Parent() != cur)
					m_Cur = m_Cur->Parent();

				// Traversed all the way to the root node - this is impossible, since
				// the iteration starts with the root and each successive fetch should
				// yield another lower-level node.  The only explanation is a node 
				// in the path was deleted during iteration or m_current/m_last is corrupt.
				assert (m_Cur->Parent() != NULL);
				assert (m_Cur != NULL);
			}

			return cur;
		}

		/****
		 *
		 * traversal for TREE_DepthFirst option
		 * Traverse the hierarchy in depth-first order, children before parents.
		 * 
		 *  input tree		traversal order
		 *
		 *		A				7
		 *	B		C		3		6
		 * D E	  F  G	  1  2	   4  5
		 *
		 * notes:
		 * On entry, m_Cur points to the lowest level child of the first
		 * group (D in our example)
		 *
		 ****/
		Tree<BASE>* DepthFirst()
		{
			Tree<BASE>* cur = m_Cur;
			Tree<BASE>* next;

			if (cur == NULL)
				return NULL;
			if (cur == m_Group)						// at the top?
			{
				m_Cur = NULL;						// indicate no more left
				return cur;
			}
			if (cur->Next())						// have a follower?
			{
				next = cur->Next();					// go to following peer
				while (next->First())				// find lowest level child
					next = next->First();
			}
			else									// back to parent
			{
				assert(cur->Parent());
				next = cur->Parent();
			}
			m_Cur = next;
			return cur;
		}
	};
	friend class Iter;

	//! Make an empty tree node.
	Tree();
	//! Dereference all children.
	virtual ~Tree();

	//! Return parent of this node, NULL if no parent.
	Tree<BASE>*			Parent()			{ return m_Parent; }
	const Tree<BASE>*	Parent() const		{ return m_Parent; }
	//! Return first child of this node, NULL if no children.
	Tree<BASE>*			First()				{ return m_First; }
	const Tree<BASE>*	First() const		{ return m_First; }
	//! Return last child of this node, NULL if no children.
	Tree<BASE>*			Last();
	const Tree<BASE>*	Last() const;
	//! Returns \b true if this node has children (is a parent).
	bool				IsParent() const	{ return m_First != NULL; }
	//! Returns \b true if this node has a parent (is a child).
	bool				IsChild() const		{ return m_Parent != NULL; }
	//! Returns \b true if this node is the last child of its parent.
	bool				IsLast() const		{ return m_Next == NULL; }
	//! Returns number of first level children.
	int					GetSize() const;
	//! Retrieves a child from a tree based on its position.
	Tree<BASE>*			GetAt(int n);
	const Tree<BASE>*	GetAt(int n) const;
	//! Retrieves a child from a group based on its position.
	Tree<BASE>*			operator[](int n)		{ return GetAt(n); }
	const Tree<BASE>*	operator[](int n) const	{ return GetAt(n); }

	//! Appends the given group as first child of this tree.
	virtual bool		PutFirst(Tree<BASE>* child);
	//! Appends the given group as last child of this tree.
	virtual bool		Append(Tree<BASE>* child);
	//! Puts this node after the given group as a sibling.
	virtual bool		PutAfter(Tree<BASE>* follow);
	//! Puts this node before the given group as a sibling.
	virtual bool		PutBefore(Tree<BASE>* before);
	//! Removes this node from its hierarchy and optionally releases it.
	virtual bool		Remove(bool free = true);
	//! Deletes all first level children. 
	virtual void		Empty();
	//! Replaces this tree with the input tree, putting it in the same place in the hierarchy.
	virtual bool		Replace(Tree<BASE>* src);

protected:
	//! Return sibling which comes before this node in the hierarchy.
	Tree<BASE>*			Prev();
	const Tree<BASE>*	Prev() const;
	Tree<BASE>*			Next()			{ return m_Next; }
	const Tree<BASE>*	Next() const	{ return m_Next; }


//	Data members
	Tree<BASE>*	m_Parent;
	Tree<BASE>*	m_First;
	Tree<BASE>*	m_Next;
};


/*!
 * @class RefTree
 * @param BASE	base class to inherit from, must be subclass of BaseObj
 *
 * @brief Building block for hierarchies of reference-counted objects.
 *
 * Each node in a tree can contain, or be a parent to,
 * any number of children. Within a hierarchy, any level of nesting is permitted
 * but circular references are not allowed. A tree node can only be a child in
 * one hierarchy - it cannot have multiple parents.
 *
 * The nodes are of a ref-tree are reference counted.
 * When a node is added, it gains a reference and its use count is incremented.
 * When a node is removed from the hierarchy, it loses a reference.
 * When a tree node ends up with no references, it is freed and each of its
 * children are dereferenced.
 *
 * Although no public access is given to the group linkage pointers,
 * you can use the VTreeIter template class to iterate over the group 
 * in a variety of ways.
 *
 * @ingroup vcore
 * @see RefObj VTreeIter Tree
 */
template <class BASE> class RefTree : public Tree<BASE>
{
public:
	RefTree();
	~RefTree();

	void	Empty();
	bool	PutFirst(Tree<BASE>* child);
	bool	Append(Tree<BASE>* child);
	bool	PutAfter(Tree<BASE>* after);
	bool	PutBefore(Tree<BASE>* before);
	bool	Remove(bool free = true);
	bool	Replace(Tree<BASE>* src);
};

} // end Core

