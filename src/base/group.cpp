/****
 *
 *
 * Group:  object that defines a hierarchy
 * Groups are the base class for Engines and Models
 *
 ****/
#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Group, SharedObj, VX_Group);

static const TCHAR* opnames[] =
{	TEXT("Remove"), TEXT("Append"), TEXT("PutBefore"),
	TEXT("PutAfter"), TEXT("SetActive"), TEXT("Replace"),
};

const TCHAR** Group::DoNames = opnames;

Group::Group(const Group& src)
{
	Copy(&src);
}

Group::~Group()
{
	Empty();
}

/***
 *
 * Removes all the children of this group
 *
 ****/
void Group::Empty()
{
	ObjectLock	lock(this);
	Group*		pred = First();
	while (pred)
	{
		if (pred->Parent())
			pred->TakeOut(this);
		pred = (Group*) pred->m_Next;
	}
	SharedTree::Empty();
}

/***
 *
 *	Removes the given child from the hierarchy it is in.
 *	The default behavior is to garbage collect this node
 *	if it is no longer referenced. The caller may request
 *	that the child be dereferenced but not deleted
 *	(its use count is decremented). If its only reference was its parent,
 *	it now has the same status as if it were returned by <<new>>.
 *	In this case, the caller is responsible for garbage collecting the node.
 *
 * Returns: Group*
 *	The object, if it was removed or NULL on error
 *
 * Also: Obj_Delete Group_PutBefore Group_PutAfter Group_Append
 *
 ****/
bool Group::Remove(bool free)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Group, GROUP_Remove) << this;
	VX_STREAM_END( )

	Group* oldpar = Parent();
	if (oldpar)
		TakeOut(oldpar);
	if (!SharedTree::Remove(free))
		return false;
	return true;
}

/***
 *
 *	Removes this node from the hierarchy it is in and puts 
 *	the source node in its place. This node is dereferenced
 *	when removed but not deleted (its use count is decremented).
 *	If its only use was in the hierarchy, it now has the same status
 *	as if it were returned by new Group(). The caller now has
 *	responsibility to garbage collect this group.
 *
 * Returns: Group*
 *	The group which was removed or NULL on error
 *
 * Also: Obj_Delete Group_Remove Group_Append
 *
 ***/
Group* Group::Replace(Group* src)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Group, GROUP_Replace) << this;
	VX_STREAM_END( )

	Group* oldpar = Parent();
	if (oldpar)
		TakeOut(oldpar);
	SharedTree::Replace(src);
	return this;

}

/***
 *
 *	The child is inserted at the end of the list of children of this group.
 *	Putting a child in a group creates another reference to the child
 *	(increments the use count). If no other outstanding references
 *	exist, the child will be garbage collected when the entire group
 *	is deleted.
 *	
 * Returns: Group*
 *	If successful, a pointer to the child that was added.
 *	A NULL return value indicates the child could not be added
 *	because it was already in another hierarchy.
 *
 ***/
Group* Group::Append(Group* child)
{
	if (child == NULL)
		return child;

	if (IsGlobal())
		GetMessenger()->Distribute(child, 0);
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Group, GROUP_Append) << this << child;
	VX_STREAM_END( )

	Group* oldpar = child->Parent();
	if (oldpar)
		child->TakeOut(oldpar);
	if (!SharedTree::Append(child))
		return NULL;
	child->PutIn((Group*) this);
    return child;
}

/***
 *
 *	The child is inserted after the given node.
 *	They will have the same parent. This is the fastest way
 *	to add something to a hierarchy.
 *	
 *	Putting a child in a group creates another reference to the child
 *	(increments the use count). If no other outstanding references
 *	exist, the child will be garbage collected when the entire group
 *	is deleted.
 *
 ***/
bool Group::PutAfter(Group* after)
{
    VX_ASSERT(after);
	VX_ASSERT((const Group*) after->m_Parent);

	if (after->m_Parent->IsGlobal())
		GetMessenger()->Distribute(this, 0);
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Group, GROUP_PutAfter) << this << after;
	VX_STREAM_END( )

	Group* oldpar = Parent();
	if (oldpar)
		TakeOut(oldpar);
	if (!SharedTree::PutAfter(after))
		return false;
	PutIn((Group*) after->m_Parent);
	return true;
}

/*!
 * @fn bool Group::PutBefore(Group* before)
 * @param before	node to become the predecessor of this node
 *
 * This node is inserted before the given node in the hierarchy.
 * They will have the same parent.
 *	
 * Putting a child in a group creates another reference to the child
 * (increments the use count). If no other outstanding references
 * exist, the child will be garbage collected when the entire group
 * is deleted.
 *
 * Putting a child in a group creates another reference to the child
 * (increments the use count). If no other outstanding references
 * exist, the child will be garbage collected when the entire group
 * is deleted.
 *
 * @see Core::RefObj
 */
bool Group::PutBefore(Group* before)
{
    VX_ASSERT(before);
	VX_ASSERT((const Group*) before->m_Parent);

	if (before->m_Parent->IsGlobal())
		GetMessenger()->Distribute(this, 0);
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Group, GROUP_PutBefore) << this << before;
	VX_STREAM_END( )

	Group* oldpar = Parent();

	if (oldpar)
		TakeOut(oldpar);
	if (SharedTree::PutBefore(before))
		return false;
	PutIn((Group*) before->m_Parent);
	return true;
}

/****
 *
 * Override for SharedObj::Copy(SharedObj*)
 *
 * Copies the attributes of one group into another. All of the
 * group's children are copied but not their visual descriptions.
 *
 ****/
bool Group::Copy(const SharedObj* src_obj)
{
	const Group* src = (Group*) src_obj;
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
    VX_ASSERT(src_obj->IsClass(VX_Group));
	if (!SharedObj::Copy(src_obj))
		return false;
	Empty();								/* dereference existing children */
	src = src->First();
	while (src)								/* clone each child in source */
	{
		Group* child = (Group*) src->Clone();
		if (child == NULL)					/* could not make instance? */
			{ VX_ERROR(("Group::Copy cannot clone source object\n"), false); }
		else
			Append(child);
		src = src->Next();					/* get next source child */
	}
	return true;
}

void Group::TakeOut(Group* parent)
{
	parent->NotifyParents(CHANGED);
}

void Group::PutIn(Group* parent)
{
	parent->NotifyParents(CHANGED);
}

/****
 *
 * Override for SharedObj::Print
 *
 * Prints a description of this hierarchy on standard output.
 *
 ****/
DebugOut& Group::Print(DebugOut& dbg, int opts) const
{
	GroupIter<Group> iter(this, Group::CHILDREN);
    const Group* g;

	if ((opts & PRINT_Children) && First())
	{
		SharedObj::Print(dbg, opts & ~PRINT_Trailer);
		while (g = iter.Next())
			g->Print(dbg, opts | PRINT_Trailer);
		return SharedObj::Print(dbg, opts & PRINT_Trailer);
	}
	return SharedObj::Print(dbg, opts);
}

/***
 *
 * Sets the given flag(s) on this group and its parents.
 *
 ***/
void Group::NotifyParents(uint32 flags)
{
	Group* g = (Group*) this;
	do
	{
		g->SetFlags(flags);
		g = g->Parent();
	}
	while (g && !g->IsSet(flags));
}

/****
 *
 * class Group override for SharedObj::Do
 *		GROUP_Remove
 *		GROUP_Append	<Group*>
 *		GROUP_PutBefore	<Group*>
 *		GROUP_PutAfter	<Group*>
 *		GROUP_SetActive	<bool>
 *
 ****/
bool Group::Do(Messenger& s, int op)
{
	SharedObj*	obj;
	Group*		group;
	int32		flag;
	Opcode		o = Opcode(op);	// for debugging

	switch (op)
	{
		case GROUP_Remove:
		Remove(Group::UNLINK_FREE);
		break;

		case GROUP_Append:
		s >> obj;
		if (obj == NULL)
			break;
		VX_ASSERT(obj->IsClass(VX_Group));
		group = (Group*) obj;
		Append(group);
		break;

		case GROUP_PutBefore:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Group));
		group = (Group*) obj;
		PutBefore(group);
		break;

		case GROUP_PutAfter:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Group));
		group = (Group*) obj;
		PutAfter(group);
		break;

		case GROUP_SetActive:	// for past compatibility (moved to SharedObj)
		s >> flag;
		if (flag)
			SetActive(true);
		else SetActive(false);
		break;

		default:
		return SharedObj::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Group::DoNames[op - GROUP_Remove]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * class Group override for SharedObj::Save
 *
 ****/
int Group::Save(Messenger& s, int opts) const
{
	int32 h = SharedObj::Save(s, opts);
	if (h < 0)
		return h;
	if (opts == Messenger::MESS_ClearGlobal)
		return h;
	GroupIter<Group> iter(this, Group::CHILDREN);
	const Group*	 child;
	while (child = iter.Next())
	{
		if ((child->Save(s, opts) >= 0) && h)
			s << OP(VX_Group, GROUP_Append) << h << child;
	}
	if (h <= 0)
		return h;
	return h;
}

/*!
 *@fn Group* Group::Find(const TCHAR* searchstr, int opts)
 * @param searchstr	search string to compare names with
 * @param opts		search options
 *
 * Looks for a node above or below this one based on matching
 * the node name against a search string. This function is
 * useful for searching the hierarchy for nodes based on
 * naming conventions in the scene graph. The default behavior
 * is to search the children breadth first for a name that
 * "loosely" matches. A "loose" match is when the object name
 * contains the search string but does not exactly match it.
 * The match options are:
 * @li Group::FIND_EXACT	require exact name match
 * @li Group::FIND_END		require match at end of name
 * @li Group::FIND_START	require match at start of name
 * @li Group::FIND_PARENT	search for parent that matches, examines no children
 * @li Group::FIND_CHILD	search for child that matches, breadth first traversal
 * @li Group::CHILDREN		only search among top level children
 *
 * @see Messenger::Find Engine::FindTarget
 */
Group* Group::Find(const TCHAR* searchstr, int opts)
{ return (Group*) ((const Group*) this)->Find(searchstr, opts); }

const Group* Group::Find(const TCHAR* searchstr, int opts) const
{
	const Group*	g = this;
	int				searchopts = opts & Tree::SEARCH_MASK;
	ObjectLock		lock(g);
	const TCHAR*	name = GetName();

//
// check to see if this shapes name matches and return self if so
// check we are only searching for active objects
//
	if ((name != NULL) &&
		(!(opts & Group::FIND_ACTIVE) || IsActive()))
	{
		if (opts & Group::FIND_EXACT)		// need exact name match?
		{
			if (STRCASECMP(name, searchstr) == 0)
				return this;				// this is the one
		}
		else if (opts & Group::FIND_END)	// match if name ends with search string
		{
			intptr n = STRLEN(name) - STRLEN(searchstr);
			if ((n >= 0) && (STRCASECMP(name + n, searchstr) == 0))
				return this;
		}
		else if (opts & Group::FIND_START)	// match if name starts with search string
		{
			intptr n = STRLEN(searchstr);
			if (STRNCASECMP(name, searchstr, n) == 0)
				return this;
		}
		else if (STRSTR(name, searchstr))	// match if name contains search string
			return this;
	}
	switch (searchopts)
	{
// Group::PARENTS_REVERSE looks for a name match on our parent chain
//
		case FIND_PARENT:
		g = Parent();
		if (g)
			return g->Find(searchstr, opts);
		return NULL;
//
// Group::FIND_CHILD looks for a name match among top level children only
// Group::FIND_DESCEND, recursively look for a match among our children
//
		case FIND_CHILD:
		opts &= ~FIND_CHILD;

		case FIND_DESCEND:
		Group::IterNotSafe iter(this, CHILDREN);
		while (g = (const Group*) iter.Next())
		{
			const Group* grp = g->Find(searchstr, opts);
			if (grp != NULL)			// found a matching one?
				return grp;
		}
	}
	return NULL;
}	

ObjArray* Group::FindAll(const TCHAR* searchstr, int opts) const
{
	ObjArray*	found = new ObjArray();
	const Group* g;
	GroupIter<Group> iter(this, Group::DEPTH_FIRST);
	while (g = iter.Next())
	{
		const TCHAR* name = g->GetName();
		if (name == NULL)
			continue;
		if (opts & Group::FIND_EXACT)		// need exact name match?
	   {
			if (STRCASECMP(name, searchstr) == 0)
				found->Append(g);	// this is one
	   }
		else if (opts & Group::FIND_END)
	   {
			intptr n = STRLEN(name) - STRLEN(searchstr);
			if ((n >= 0) && (STRCASECMP(name + n, searchstr) == 0))
				found->Append(g);
	   }
		else if (STRSTR(name, searchstr))	// match if name contains search string
			found->Append(g);
	}
	if (found->GetSize() > 0)
		return found;
	found->Delete();
	return NULL;
}

}	// end Vixen