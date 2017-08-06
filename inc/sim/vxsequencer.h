#pragma once


/*!
 * @file vxsequencer.h
 * @brief Maintains a set of animation sequences for a hierarchy.
 *
 * The sequencer can load one or more animation sequences and
 * and schedule them by name. The animations can control an
 * entire hierarchy and can be blended together.
 *
 * @author Nola Donato
 * @ingroup visen
 *
 * @see vxanimator.h vxscriptor.h
 */

namespace Vixen {

/*!
 * @class Sequencer
 *
 * @brief A framework for asynchronously loading animation files and applying them to a hierarchy.
 *
 * Animations can be loaded, started, stopped and even blended.
 * Animations are stored in Vixen binary format. This is the same format exported
 * from Maya and used to distribute scene manager changes over a network.
 * Depending on the type of animation the file may contain an event stream or
 * a set of animation engines and data.
 *
 * Animations are identified by name and controlled using the Animator class.
 * For each animation, the scriptor has a Animator child engine to control it.
 * An animation can be any hierarchy of engines that operates on one or more
 * target objects. A camera path or a skinned character are examples of animations.
 * For each animation, the sequencer appends an instance of Animator
 * as a child engine. The animator classes allow you to control a hierarchy of
 * engines anywhere in the scene (they need not be children of the animator).
 *
 * @see Animator BoneAnimator
 */
class Animator;
class Sequencer : public Engine
{
public:
	VX_DECLARE_CLASS(Sequencer);

	//! Create an animator to control the animation loaded from the input file.
	virtual Animator*	Load(const TCHAR* file, const TCHAR* engname = NULL, int opts = 0, SharedObj* targ = NULL);

	//! Disable all animator children belonging to this scriptor.
	virtual void	Empty();

	//! Start the named animation belonging to this scriptor.
	virtual	void	Begin(const TCHAR* animname, const TCHAR* follow = NULL, SharedObj* target = NULL, int playopts = 0);
	virtual void	Begin();

	//! Stop the named animation belonging to this scriptor.
	virtual	void	End(const TCHAR* animname);
	virtual void	End();

	//! Stop the named animation and detach it from this scriptor.
	virtual	void	Kill(const TCHAR* animname = NULL);

	//! Return the hierarchy controlled by the named animation.
	Engine*			GetRootEngine(const TCHAR* name) const;

	//! Return the animator controlling the named animation.
	Animator*		FindAnim(const TCHAR* name, SharedObj* targ = NULL) const;

	Animator*		MakeAnim(const TCHAR* animname, SharedObj* target = NULL, bool domerge = false);

	// Overrides
	virtual bool	Eval(float t);
	virtual	bool	OnStop();
	virtual bool	Do(Messenger& s, int op);

	/*
	 * Sequencer::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		SEQ_Load = Engine::ENG_NextOp,
		SEQ_Empty,
		SEQ_Begin,
		SEQ_Kill,
		SEQ_End,
		SEQ_NextOp = Engine::ENG_NextOp + 30
	};

protected:
	Engine*		FindLast(Animator* eng);
	void		LoadSound(const TCHAR* filename, SharedObj* targobj, int opts);
	void		LoadVideo(const TCHAR* filename, SharedObj* targobj, int opts);
};

#define SEQ_FollowLast		((Engine*) -1)

} // end Vixen
