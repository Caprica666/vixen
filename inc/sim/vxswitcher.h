/*!
 * @file vxswitcher.h
 * @brief Switches state based on time.
 *
 * The switcher updates a state index in its target based on
 * a vector of elapsed times.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxtexswitcher.h
 */
#pragma once

namespace Vixen {

/*!
 * @class Switcher
 * @brief Engine that sets an index in it's target object
 * based on the current time.
 *
 * The Switcher internally keeps a set of times at which to advance the index and an
 * optional callback function for each time that is invoked
 * when the index changes. You can also advance at fixed time intervals.
 * It is used in conjunction with other objects like ModelSwitch and ImageSwitch
 * which can select a mesh or texture based on an dynamically changing index.
 *
 * @see Engine ModelSwitch ImageSwitch
 */
class Switcher : public Engine
{
public:
	VX_DECLARE_CLASS(Switcher);

	Switcher();
	int		GetSize() const;			//! Return size of time vector.
	int		GetIndex() const;			//!< Return current switch index.
	float	GetTimeSpan() const;		//!< Return time between index changes.
	void	SetSize(int n);				//!< Set size of time vector.
	void	SetTimeSpan(float);			//!< Set number of seconds between index changes.
	bool	Add(float t);				//!< Append to time vector.
	float	GetAt(int i) const;			//!< Return time vector entry based on index.
	void	SetAt(int i, float time);	//!< Set time vector entry.

	virtual void	OnIndexChange(int newindex);
	virtual bool	Eval(float t);
	virtual bool	OnReset();
	virtual bool	OnStart();
	virtual bool	Copy(const SharedObj*);
	virtual bool	Do(Messenger& s, int op);
	virtual int		Save(Messenger&, int) const;
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*
	 * Switcher::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		SWITCHER_SetSize = Engine::ENG_NextOp,
		SWITCHER_SetAt,
		SWITCHER_Add,
		SWITCHER_UNSUED0,
		SWITCHER_UNUSED1,
		SWITCHER_SetTimeSpan,
		SWITCHER_Empty,
		SWITCHER_NextOp = Engine::ENG_NextOp + 20
	};

protected:
	int32			m_Index;	// current index
	float			m_TimeSpan;	// time span if no time array
	FloatArray	m_Time;		// time vector
	float           m_Lastt;
};

inline int Switcher::GetIndex() const
{	return m_Index; }

inline int Switcher::GetSize() const
{	return m_Time.GetSize(); }

inline float Switcher::GetTimeSpan() const
{	return m_TimeSpan; }

} // end Vixen