/*
 * SWIG interface file for Box2, Box3
 */
namespace Vixen
{

	%ignore	VX_GROUP_DECLARE;

	namespace Core
	{
		%ignore List;
		%ignore Tree::operator[];
		%ignore Tree::Parent() const;
		%ignore Tree::First() const;
		%ignore Tree::Last() const;
		%ignore Tree::GetAt(int) const;
	}
	%include "vcore/vtree.h"

	%ignore Group::Iter;
	%ignore Group::TakeOut;
	%ignore Group::PutIn;
	%ignore Group::NotifyParent;
	%ignore Group::operator[];
	%ignore Group::First() const;
	%ignore Group::Last() const;
	%ignore Group::GetAt(int) const;
	%ignore Group::Parent() const;
	%ignore Group::Find(const TCHAR*, int) const;
	%ignore Group::Find(const TCHAR*) const;
	%ignore Group::Next() const;
	%ignore GroupIter;
	%typemap(csclassmodifiers)	CoreTree	"internal class";
	%typemap(csclassmodifiers)	SharedTree	"internal class";
	%template(CoreTree)	Core::Tree<SharedObj>;
	%template(SharedTree)	Core::RefTree<SharedObj>;

	%apply SharedObj* { Group* };
}

%include "base/vxgroup.h"




