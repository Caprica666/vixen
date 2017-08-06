/*!
 * @file vxgroup.h
 * @brief Serializable, thread safe hierarchy class.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vtree.h
 */

#pragma once

namespace Vixen {

class Messenger;

typedef Core::RefTree<SharedObj>	SharedTree;

/****
 *
 * VX_GROUP_DECLARE(class)
 *
 * Declares convenient overrides for Group accessors
 *
 ****/
#define VX_GROUP_DECLARE(CLS)  									\
    const CLS*	Parent() const			{ return (const CLS*) Core::Tree<SharedObj>::Parent(); } \
    const CLS*	First() const			{ return (const CLS*) Core::Tree<SharedObj>::First(); } \
    const CLS*	Last() const			{ return (const CLS*) Core::Tree<SharedObj>::Last(); } \
    const CLS*	Next() const			{ return (const CLS*) Core::Tree<SharedObj>::Next(); } \
	const CLS*	GetAt(int i) const		{ return (const CLS*) Core::Tree<SharedObj>::GetAt(i); } \
	const CLS*	operator[](int i) const	{ return (const CLS*) Core::Tree<SharedObj>::GetAt(i); } \
    CLS*		Parent()				{ return (CLS*) Core::Tree<SharedObj>::Parent(); } \
    CLS*		First()					{ return (CLS*) Core::Tree<SharedObj>::First(); } \
    CLS*		Last()					{ return (CLS*) Core::Tree<SharedObj>::Last(); } \
    CLS*		Next()					{ return (CLS*) Core::Tree<SharedObj>::Next(); } \
	CLS*		GetAt(int i)			{ return (CLS*) Core::Tree<SharedObj>::GetAt(i); } \
	CLS*		operator[](int i)		{ return (CLS*) Core::Tree<SharedObj>::GetAt(i); }


/*!
 * @class Group
 * @brief Serializable thread-safe hierarchy with name search.
 *
 * This is a generalized tree class that is used as the base for the scene graph
 * and the simulation tree. A group can contain, or be a parent to, any number of children.
 * A parent group and its children form a hierarchy. Within this hierarchy,
 * any level of nesting is permitted but no circular references are allowed.
 * A group node can only be a child to one group - it cannot have multiple
 * parents (so it is not really a scene "graph").
 *
 * Groups are reference counted. When a group is placed in a hierarchy,
 * it gains a reference. When it is removed from the hierarchy,
 * it loses a reference. Deleting a group with no outstanding references
 * frees the group and the resources it uses.
 *
 * Although no public access is given to the group linkage pointers,
 * you can use the Group::Iter template class to iterate over the group 
 * in a variety of ways.
 *
 * The messenger knows how to load and save groups in an efficient way.
 * Each node is saved only once, the first time it is referenced.
 * Saving a group saves all of its children, too.
 *
 * @see Model Engine Group::Iter Tree
 */
class Group : public SharedTree
{
public:
	VX_DECLARE_CLASS(Group);

	/*!
	 * @brief Values for iterating and finding
	 *
	 * @see Find Group::Iter Group::Remove Group::Find
	 */
	enum
	{
		DEPTH_FIRST = SharedTree::DEPTH_FIRST,			//!< search all descendants depth first (Iter)
		PARENTS_REVERSE = SharedTree::PARENTS_REVERSE,	//!< search from this node thru all parents (Iter)
		CHILDREN = SharedTree::CHILDREN,				//!< search only top level children first to last (Iter, Find)
		CHILDREN_REVERSE = SharedTree::CHILDREN_REVERSE,//!< search top level children in reverse, last to first (Iter)
		FIND_DESCEND = SharedTree::DEPTH_FIRST,
		FIND_PARENT = PARENTS_REVERSE,	//!< search parents for name match (Find)
		FIND_CHILD = CHILDREN,			//!< search children for name match (Find)
		FIND_EXACT = 32,				//!< require exact match for name search (Find)
		FIND_END = 64,					//!< search for match at end of name (Find)
		FIND_ACTIVE = 128,				//!< search for active objects only (Find)
		FIND_START = 256,				//!< search for match at start of name (Find)
		UNLINK_FREE = 1,				//!< unlink and free (Remove)
		UNLINK_NOFREE = 0				//!< unlink but do not free (Remove)
	};

	/*!
	 * @brief Iterator used to traverse groups in a thread-safe way.
	 *
	 * The constructor initializes the iterator to start
	 * at a specific root node. Each time the \b Next function is called,
	 * a pointer to a different node from the hierarchy is returned.
	 * Several hierarchy traversal options are provided.
	 *
	 * The root node is when the traversal starts.
	 * Nodes are locked as they are visited. The previously locked
	 * node is unlocked when a new node is visited.
	 *
	 * @todo Support changing of hierarchy during iteration.
	 *
	 * @see Group Core::Tree::Iter GroupIter
	 */
	typedef Core::RefTree<SharedObj>::Iter IterNotSafe; 
	class Iter : public IterNotSafe
	{
	public:
		Iter(Group* group, int opts = Group::DEPTH_FIRST) 
		: IterNotSafe(group, opts), m_Lock(group) 
		{ }

		Iter(const Group* group, int opts = Group::DEPTH_FIRST) 
		: IterNotSafe(group, opts), m_Lock(group) 
		{ }

		Group*	Current()	{ return (Group*) m_Cur; }
		Group*	Next()		{ return (Group*) (this->*m_Func)(); }
		Group*	Next(uint32 classid)
		{
			Group* next;
			while (next = (Group*) (this->*m_Func)())
				if (next->IsClass(classid))
					return next;
			return NULL;	
		}

protected:
		ObjectLock	m_Lock;
	};

	/*
	 * Group::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		GROUP_Remove = SharedObj::OBJ_NextOp,
		GROUP_Append,
		GROUP_PutBefore,
		GROUP_PutAfter,
		GROUP_SetActive,
		GROUP_Replace,
		GROUP_NextOp = SharedObj::OBJ_NextOp + 20,
	};

	VX_GROUP_DECLARE(Group);

//! Construct a clone of the input group, cloning all children.
	Group(const Group& src);

//!	Construct an empty group with no children.
	Group() { }

//! Delete all the items in the group
	~Group();

//! Finds a child in the hierarchy whose name matches the search string.
	Group*			Find(const TCHAR* searchstr, int opts = Group::FIND_CHILD);
	const Group*	Find(const TCHAR* searchstr, int opts = Group::FIND_CHILD) const;

//! Find all children whose names match the search string.
	ObjArray*		FindAll(const TCHAR* searchstr, int opts) const;

//	Hierarchy overrides
	Group*			Append(Group* child);
	bool			PutAfter(Group* after);
	bool			PutBefore(Group* before);
	bool			Remove(bool free = true);
	void			Empty();
	Group*			Replace(Group*);

//! Called when this group is removed from a hierarchy.
	virtual	void	TakeOut(Group* parent);

//! Called when this group is put into a hierarchy.
	virtual	void	PutIn(Group* parent);
	virtual void	NotifyParents(uint32);


// Serialization overrides
	bool			Do(Messenger& s, int opcode);
	int				Save(Messenger&, int) const;
	bool			Copy(const SharedObj* src);
	DebugOut&		Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;
};

/*!
 * @class GroupIter
 * @brief Iterator used to traverse groups in a thread-safe way.
 * @param T	type of node to return from iterator
 *
 * The constructor initializes the iterator to start
 * at a specific root node. Each time the \b Next function is called,
 * a pointer to a different node from the hierarchy is returned.
 * Several hierarchy traversal options are provided.
 *
 * @see Group Group::Iter Core::Tree::Iter
 */
template <class T> class GroupIter : public Group::Iter
{
public:
	GroupIter(T* group, int opts = Group::DEPTH_FIRST)
	: Group::Iter(group, opts) { }

	GroupIter(const T* group, int opts = Group::DEPTH_FIRST)
	: Group::Iter(group, opts) { }

	T*		Next()		{ return (T*) (this->*m_Func)(); }
	T*		Next(uint32 classid)
	{
		T*	next;
		while (next = (T*) (this->*m_Func)())
			if (next->IsClass(classid))
				return next;
		return (T*) NULL;	
	}
};

template <class T> class GroupIterNotSafe : public Group::IterNotSafe
{
public:
	GroupIterNotSafe(T* group, int opts = Group::DEPTH_FIRST)
	: Group::IterNotSafe(group, opts) { }

	GroupIterNotSafe(const T* group, int opts = Group::DEPTH_FIRST)
	: Group::IterNotSafe(group, opts) { }

	T*		Next()		{ return (T*) (this->*m_Func)(); }
};

} // end Vixen
