#pragma once

/*!
 * @file vxscriptor.h
 * @brief Executes simple text scripts that control animation.
 *
 * The Vixen scripting language allows you to load animations from
 * files and control them with text scripts.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxanimator.h vxsequencer.h
 */
namespace Vixen {

#define	SCRIPT_Key	5

class Scriptor;

/*!
 * @class ScriptParser
 * @brief Internal class to parse the very simple Vixen scripting language
 *
 * @see Scriptor
 */
class ScriptParser : public SharedObj
{
public:
	ScriptParser();
	int					Exec(const TCHAR* scriptdata);
	bool				WhenEvent(int code, SharedObj* sender, const TCHAR* s);
	bool				OnEvent(Event*);
	static SharedObj*	FindObj(const TCHAR* name);
	static Model*		FindModel(const TCHAR* name, bool sceneonly = false);
	static Engine*		FindEngine(const TCHAR* name, bool sceneonly = false);

	Scriptor*	Scriptor;	
	int			Error;

	enum ScriptErr
	{
		SCRIPT_Error = -1,
		SCRIPT_ErrorBadCommand = -2,	// illegal script command
		SCRIPT_ErrorBracket = -3,		// mismatched brackets
		SCRIPT_ErrorBadEvent = -4,		// illegal event name
		SCRIPT_ErrorObjName = -5,		// illegal object name
		SCRIPT_ErrorNotFound = -6,		// object not found
		SCRIPT_ErrorEOF = -7,			// premature EOF
		SCRIPT_ErrorBadKey = -9,		// illegal keycode
	};

protected:
	const TCHAR*	ExecLine(const TCHAR*);
	const TCHAR*	ParseLoad(const TCHAR* s);
	const TCHAR*	ParseBegin(const TCHAR* s);
	const TCHAR*	ParseEnd(const TCHAR* s);
	const TCHAR*	ParseDisable(const TCHAR* s);
	const TCHAR*	ParseEnable(const TCHAR* s);
	const TCHAR*	ParseCamera(const TCHAR* s);
	const TCHAR*	ParseDistribute(const TCHAR* s);
	const TCHAR*	ParseKill(const TCHAR* s);
	const TCHAR*	ParseOnEvent(const TCHAR* s);
	const TCHAR*	ParseOnPick(const TCHAR* s);
	const TCHAR*	ParseOnHit(const TCHAR* s);
	const TCHAR*	ParseOnKey(const TCHAR* s);
	const TCHAR*	ParseFile(const TCHAR* s, TCHAR* namebuf, int& type);
	const TCHAR*	ParseName(const TCHAR* s, TCHAR* namebuf);
	const TCHAR*	ParseEventCode(const TCHAR* s, int32& code);
	const TCHAR*	MakeEventString(int code);
	void			ExpandName(TCHAR* namebuf);
	bool			HandleEvent(Event* event, const TCHAR* s);
	bool			HandleHit(TriggerEvent* event, const TCHAR* s);
	bool			HandlePick(Event* event, const TCHAR* s);
	const TCHAR*	SkipToEnd(const TCHAR* s);
	const TCHAR*	MakeScript(const TCHAR* s, TCHAR* buf, int code);
	const TCHAR*	SkipComments(const TCHAR* s);

	Event		m_Event;
};

/*!
 * @class Scriptor
 *
 * @brief Asynchronously loads animation files and applyies them to a target hierarchy.
 *
 * Animations can be loaded, started, stopped and even blended using
 * a simple scripting language. Scripts can be loaded from a
 * file or set from an ASCII string.
 *
 * Animations are identified by name and can be any group of engines that controls
 * a hierarchy of models whose root is the target of this scriptor.
 * A camera path or a skinned character are examples of animations.
 *
 * For each animation, the scriptor appends an instance of Animator
 * as a child engine. The animator classes allow you to control a hierarchy of
 * engines anywhere in the scene (they need not be children of the animator).
 *
 * <B>Scripting Language Commands</B>
 *	The Vixen scripting language is used for simple animation control within a scene.
 *	You can start and stop animations, blend them or schedule animation sequences.
 *	Scripts can be initiated at startup or in response to events such as picking or
 *	object collisions. Detailed documentation for the scripting language can be
 *	found in the @htmlonly <A><HREF=oview/animoview.htm> Animation Overview</A> @endhtmlonly
 *
 * @code
 *	Command		Description
 *	load		load script or animation
 *	begin		start animation
 *	end			stop animation
 *	enable		continue animation / show model
 *	disable		pause animation / hide model
 *	kill		kill animation (stop and remove)
 *	onevent		respond to event
 *	onhit		respond to collision
 *	onpick		respond to pick
 *	onkey		respond to keypress
 *	exec		execute script
 *	distribute	specify distribution options
 * @endcode
 *
 * <B>bject Naming</B>
 * Scene objects and engines are referenced by name in the script.
 * The scene manager art exporter gives models the same names as the artist gave them
 * except that it prepends the base name of the output file. A model
 * called "mybox" in the file "c:/data/foo.vix" would become "foo.mybox".
 *
 * Engines are named after their target object with a suffix indicating their
 * usage in the scene. A transform controller for object "foo.sphere" would be named "foo.sphere.xform".
 *
 * @code
 *	Suffix		Engine			Description
 *	.picker		NamePicker		object picker
 *	.xform		Transformer		Transform controller
 *	.skeleton	Skeleton		Skeleton controller
 *	.skin		Skin			Skinning engine
 *	.deformer	Deformer		Morphing engine
 *	.pos		KeyFramer		position keyframer
 *	.rot		KeyFramer		rotation keyframer
 *	.scl		KeyFramer		scaling keyframer
 *	.pos.blend	KeyFramer		position keyframe blender
 *	.rot.blend	KeyFramer		rotation keyframe blender
 *	.anim		Animator		animation sequencer
 *	.scriptor	Scriptor		scripting engine
 *	.sound		SoundPlayer		sound source
 *	.mpg		MediaSource		MPEG video decodeer
 *	.ifl		ImageSwitch		texture animation
 *	.flyer		Flyer			airplane navigator (flying)
 *	.navigator	Navigator		car navigator (driving)
 * @endcode
 *
 * <B>Animation Sequencing</B>
 * Animation engines can be loaded from files or created programatically.
 * In a script, you can reference an animation engine by the name
 * it has in the scene. The animation sequencer can queue animations to
 * start immediately, or when a designated animation finishes.
 *
 * @code
 *	load <file> <engine> -c -s -t <target> -start <time> -duration <time>
 *	begin <anim> <start> <duration> -r -c -f <follow> -t <target>
 *	end <anim>
 *	kill <anim>
 * @endcode
 *
 * Arguments to  begin and  load control the various animation options.
 * The  -start argument indicates when to start the animation. It is
 * relative to the time the animation was scheduled so negative times
 * make sense when blending animations. A start time of zero will start the
 * animation as soon as possible. If the animation is designated to follow
 * another, it starts as soon as its predecessor finishes. A time of 1 sec
 * will start 1 sec after the predecessor. A time of -1 sec will start
 * 1 sec before the predecessor finishes so that animations may be
 * blended together.
 * @code
 *	-c -cycle	cycle (restart animation when it stops)
 *	-r	reverse animation
 *	-f -follow	follow specified animation
 *	-t -target	set target for animation
 *	-s -suspend	suspend animation, do not automatically start
 *	-start		time to start animation
 *	-duration	how long to run animation
 *	-speed		how fast to go
 *	-blend		time to blend between animations
 *	-blendto	time to blend into next animation
 * @endcode
 *
 * <B>Event Handling</B>
 * The onevent command causes the script commands that follow
 * to be executed only if a particular event is generated by a
 * given animation engine. Similarly, onhit will execute
 * only if the two designated objects collide.
 * The script below will start animations foo.door1 and foo.creak
 * whenever any object collides foo.hotspot1. When the foo.door1
 * animation stops, a  stop event is sent which will load
 * the script  scripts/room1.scp.
 *
 * @code
 *	onhit foo.hotspot1 { begin foo.door1; begin foo.creak }
 *	onevent stop foo.door1, load scripts/room2.scp
 * @endcode
 *
 * <B>Distribution</B>
 * The  distribute command indicates how an object or class should be distributed.
 * If a  class is given, the distribution options are applied to all objects in
 * this class.
 * @code
 *	distribute <objname> -s -g -d -n
 *	distribute class <classname> -s -g -d -n
 * @endcode
 * The arguments control whether an object is  shared,  global,  inactive or
 * not distributed at all.
 * @code
 *	-s	share with remote clients
 *	-g	share and transmit all changes to remote clients
 *	-d	disable when received by a remote client
 *	-n	do not distribute at all
 * @endcode
 *
 * @see Sequencer Animator BoneAnimator ScriptParser
 */
class Scriptor : public Sequencer
{
	friend class ScriptParser;
public:
	VX_DECLARE_CLASS(Scriptor);

	//! Construct an empty scriptor with no animations.
	Scriptor();

	//! Get most recent error encountered during scripting.
	int				GetError() const			{ return m_Parser.Error; }

	//! Get base directory for script files loaded from disk.
	const TCHAR*	GetDirectory() const	{ return m_Directory; }

	//! Set base directory for script files loaded from disk.
	void			SetDirectory(const TCHAR*);

	//! Find an object in the scene by name.
	static SharedObj*	Find(const TCHAR* name);

	//! Execute the script in the input string.
	virtual void	Exec(const TCHAR* scriptdata = NULL);

	//! Attach a script to the sender that is executed when the sender logs the given event code.
	virtual bool	WhenEvent(int code, SharedObj* sender, const TCHAR* s);

	//! Asynchronously load a script file and execute it.
	virtual void	LoadScript(const TCHAR* file);

	// Overrides
	virtual bool	Do(Messenger& s, int op);
	virtual bool	OnEvent(Event*);
	virtual bool	Copy(const SharedObj*);
	virtual int		Save(Messenger&, int) const;

	/*
	 * Scriptor::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		SCRIPT_Exec = SEQ_NextOp,
		SCRIPT_LoadScript,
		SCRIPT_SetDirectory,
		SCRIPT_NextOp = SEQ_NextOp + 10
	};

	static	void	SetAnimationScale(float);
	static	float	GetAnimationScale()	{ return s_AnimScale; }

protected:
	static	float	s_AnimScale;
	Core::String	m_Directory;
	Core::String	m_FileName;
	ScriptParser	m_Parser;
};

} // end Vixen
