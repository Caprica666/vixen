#pragma once

/*!
 * @file vxanimator.h
 * @brief Classes to control animation.
 *
 * These classes allow you to load, start, stop and blend animations
 * over a hierarchy of models.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxscriptor.h vxsequencer.h
 */

namespace Vixen {

/*!
 * @class Animator
 *
 * @brief Engine that controls an engine hierarchy elsewhere in the scene.
 *
 * Operations on the animator are applied to the controlled engines even though they
 * are not children of the animator. The animator can also load a set of
 * engines from a file into the scene.
 *
 * Animations are saved in Vixen binary format. This is the same format exported
 * from Maya and used to distribute scene manager changes over a network.
 * Depending on the type of animation the file may contain an event stream or
 * a set of animation engines and data.
 *
 * The scene manager provides a scripting language to control the operation of Animator.
 * The Scriptor class provides a framework for employing multiple animators.
 * See @htmlonly <A><HREF="oview/animoview.htm">Vixen Animation Overview</A> @endhtmlonly
 * for details.
 *
 * @see Transformer BoneAnimator Sequencer Scriptor
 */
class Animator : public Engine
{
public:
	friend class Sequencer;	// for m_StateFlags
	VX_DECLARE_CLASS(Animator);

	/*!
	 * @brief general animator options.
	 * @see SetOptions
	 */
	enum
	{
		AUTO_PLAY =		2,
		LOAD_STREAM =	4,	
		KILL_ON_STOP =	128,
		ISATTACHED =	2,		//!< animator is attached to an engine hierarchy
		FROM_FILE =		4,		//!< animator engines come from file
		ISLOADING =		8,		//!< animator engines are loading
		WAS_KILLED =	16,		//!< animator was killed
		MERGED =		32,		//!< merged animation hierarchy
		PLAY_AFTER =	64,		//!< play this animation AFTER it's leader
		PLAY_WITH	=	128,	//!< play this animation WITH it's leader
		PLAY_RELATIVE =	512,	//!< play relative to current position
		PLAYOPTS =		PLAY_WITH | PLAY_AFTER,
	};

	Animator(const TCHAR* name = NULL);
	virtual ~Animator();
//
// Accessors
//
	//! Return base name of this animation engine.
	const TCHAR*	GetEngineName(TCHAR* namebuf) const	{ return m_EngineName; }

	//! Set name of source engine to use.
	void			SetEngineName(const TCHAR* name);

	//! Return name of animation file.
	const TCHAR*	GetFileName() const		{ return m_FileName; }

	//! Return root engine of controlled hierarchy.
	Engine*			GetRootEngine() const	{ return m_DstRoot; }

	//! Set options for this animation.
	void			SetOptions(int opts);

	//! Return animation options.
	int				GetOptions() const		{ return m_Options; }

	//! Set name of file to load animation from.
	bool			SetFileName(const TCHAR* filename);

	void			SetRootEngine(Engine* root);

	//! Designate an animator for this one to follow.
	void			SetLeader(const Engine*, int playopts = Animator::PLAY_AFTER);

	//! Return animator which this one follows, NULL if no leader
	Engine*			GetLeader() const;

	//! Remove animation from hierarchy.
	virtual void	Kill();

	//! Load animation from file file.
	void			Load(const TCHAR* animfile, int opts = 0);

	//! Blend results of multiple animations.
	virtual void	Blend(float time, int opts = Engine::BLEND_BETWEEN);

	const Engine*	Init(bool play);
	const TCHAR*	MakeName(const TCHAR* engname);
	int				GetState() const	{ return m_StateFlags; }
	static	TCHAR*	GetPrefix(TCHAR* buf, const TCHAR* fullname);

	/*
	 * Animator::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		ANIM_Load = Engine::ENG_NextOp,
		ANIM_Kill,
		ANIM_SetOptions,
		ANIM_SetFileName,
		ANIM_SetRootEngine,
		ANIM_SetLeader,
		ANIM_Blend,
		ANIM_SetState,
		ANIM_SetEngineName,
		ANIM_NextOp = Engine::ENG_NextOp + 30
	};

//
// Overrides
//
	virtual bool	Eval(float t);
	virtual void	Start(int = 0);
	virtual void	Stop(int = 0);
	virtual bool	OnStop();
	virtual bool	OnStart();
	virtual bool	OnReset();
	virtual void	SetControl(int flags, int who = 0);
	virtual void	Enable(int flags, int who = 0);
	virtual void	Disable(int flags, int who = 0);
	virtual void	SetStartTime(float t, int who = 0);
	virtual void	SetTarget(SharedObj* target);
	virtual bool	Do(Messenger& s, int op);
	virtual bool	OnEvent(Event*);
	virtual bool	Copy(const SharedObj*);
	virtual int		Save(Messenger&, int) const;
	virtual float	ComputeDuration(Engine* root);

protected:
	static	TCHAR*	GetTargetName(TCHAR* buf, const TCHAR* fullname);
	virtual	void	OnBlend();
	virtual Engine*	AttachEngine(Engine*);
	Engine*			FindSrcRoot(const Engine*);

	int32			m_Options;
	int32			m_StateFlags;
	int32			m_AnimClass;
	int32			m_WhenStart;
	float			m_TimeStart;
	float			m_MaxDur;	
	float			m_BlendTime;
	Core::String	m_FileName;
	Core::String	m_EngineName;
	Ref<Engine>		m_SrcRoot;
	Ref<Engine>		m_DstRoot;
	Ref<Messenger>	m_Stream;
	WeakRef<Engine>	m_Leader;
};

/*!
 * @class BoneAnimator
 *
 * Loads and controls multiple animation files that apply to a single animated character.
 * The target of the animator is assumed to be a skinned character with an
 * underlying bone structure. Each bone has a corresponding transformer engine that controls its
 * position and rotation. Usually, the bone hierarchy is constructed the same way as
 * the model hierarchy it applies to.
 *
 * Animations files exported from Maya (as .Vix files) use a naming convention
 * to indicate which engines control which targets. All scene manager object names contain the
 * filename followed by the model or target name. Engine names also append the type of
 * engine. For example, a Maya object  fish with a transform controller exported to
 * file billy.vix will be called billy.fish. The simulation tree for the scene will
 * contain a Transformer called billy.fish.xform with  fish as its target.
 *
 * To attach a newly loaded animation hierarchy to an existing bone hierarchy,
 * target bones are attach to transformers which contain their name. The first animation
 * loaded establishes the structure of the initial hierarchy. Loading subsequent animations
 * does not create a separate hierarchy. Instead, the new animations are "blended" into
 * the existing hierarchy by putting children of the new transformers under corresponding
 * parents in the existing structure. This blending is done based on matching target names.
 *
 * Once an animation is loaded, it is referenced by the name of the file it came from.
 * (not including the directory and extension). Once loaded, animations can be started,
 * stopped or deleted. Once constructed, the basic transformer structure remains in place.
 * Only child engines whose filename matches the animation name are affected.
 *
 * <B>Blending</B>
 * The bone animator can smoothly transition between entire animation hierarchies.
 * Blending is supported at the lowest level by the KeyBlender class which can
 * alpha blend two keyframe animation tracks. These blenders are inserted into the
 * hierarchy when multiple keyframers for the same target are encountered.
 *
 * @see Animator KeyBlender
 */
class BoneAnimator : public Animator
{
	friend class Scriptor;

	/*!
	 * @class Iter
	 *
	 * Iteratively returns all of the animation engines which are part of
	 * the named animation. Usually the name of an animation is the base
	 * filename of the file it came from (without extension and directory).
	 *
	 * @ingroup vixen
	 */
	class Iter : public GroupIter<Engine>
	{
	public:
		Iter(BoneAnimator*);
		Iter(Engine* e) : GroupIter<Engine>(e) { };
		Engine*	Next();
		Engine*	Next(int classid);

	protected:
		Core::String	m_SearchString;
	};

public:
	VX_DECLARE_CLASS(BoneAnimator);
	BoneAnimator(const TCHAR* name = NULL);
	BoneAnimator(const BoneAnimator&);
	~BoneAnimator();

	virtual void	Kill();
	virtual void	SetTarget(SharedObj* target);
	virtual void	Start(int = 0);
	virtual void	Stop(int = 0);
	virtual bool	OnStop();
	virtual void	SetControl(int flags, int who = 0);
	virtual void	Enable(int flags, int who = 0);
	virtual void	Disable(int flags, int who = 0);
	virtual void	Blend(float time, int opts = Engine::BLEND_BETWEEN);

protected:
	virtual void	OnBlend();
	void			AttachBlenders(Engine* dstroot);
	Engine*			AttachEngine(Engine*);
	int				AttachTarget(Engine*, SharedObj*);
	void			Merge(Engine* dst, Engine* src);
	void			AddKeyFramer(Engine* dsteng, Engine* srceng);
	void			StartEngine(Engine* eng, int who = 0, bool cycle = false);
};

} // end Vixen