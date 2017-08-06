#pragma once

namespace Vixen {
/*!
 * @class FrameStats
 * @brief Engine that gathers statistics during a single frame.
 *
 * Statistics can be displayed in a string, with one line
 * for each enabled property. If the target of the status
 * log engine is a TextGeometry, the status string is displayed
 * in that image.
 *
 * Statistics may be averaged over a period of time established
 * by the  duration of the engine. If the duration is non-zero,
 * the values of each property are averaged over that
 * period and the statistics display is updated then. Otherwise, the
 * average value is always the same as the current value and
 * the display is updated every frame.
 *
 * You can add new properties that are treated the same as the default ones
 * with respect to enable, disable and display. Your application is responsible
 * for gathering information and setting added properties.
 * They are not gathered automatically like the default ones are. 
 *
 * @par BuiltIn Properties
 * @code
 *	STAT_FrameRate		frames processed per second
 *	STAT_VertsCulled	number of vertices culled this frame
 *	STAT_VertsRendered	number of vertices rendered this frame
 *	STAT_StateChanges	render state changes this frame
 *	STAT_PrimsRendered	primitives rendered this frame
 *	STAT_ModelsRendered	number of models rendered this frame
 *	STAT_ModelsCulled	number of models culled this frame
 *	STAT_FrameTime		time in seconds to process this frame
 * @endcode
 *
 * @see TextGeometry Engine::SetDuration
 */
enum StatOps
{
	STAT_SetValue = Engine::ENG_NextOp,
	STAT_AddValue,
	STAT_Enable,
	STAT_Disable,
	STAT_NextOp = Engine::ENG_NextOp + 10
};

enum psStatID
{
	STAT_FrameRate = 0,
	STAT_VertsCulled,
	STAT_VertsRendered,
	STAT_StateChanges,
	STAT_PrimsRendered,
	STAT_ModelsRendered,
	STAT_ModelsCulled,
	STAT_FrameTime,
	STAT_LastProp = STAT_FrameTime,

//	room for user-added properties
	STAT_MaxProp = STAT_LastProp + 20
};

#define	STAT_MaxString	1024

class FrameStats : public Engine
{
public:
	VX_DECLARE_CLASS(FrameStats);
	FrameStats();
	~FrameStats();

	struct StatProp
	{
		StatProp()		{ Enable = false; Name = NULL; Reset(); }
		void Reset()	{ Value = Average = Minimum = Maximum = 0; Samples = 0; }

		bool		Enable;		// enable / disable
		float		Value;		// current value
		float		Average;	// average value
		float		Minimum;	// minimum value
		float		Maximum;	// maximum value
		int			Samples;	// # samples taken
		const TCHAR* Name;		// string name
	};

	virtual bool	Eval(float t);
	virtual	bool	OnReset();
	virtual	bool	OnStart();

	const TCHAR*	GetString() const;				//!< Return statistics string.
	float			GetAverage(int prop) const;		//!< Return average value of property.
	float			GetMinimum(int prop) const;		//!< Return minimum value of property.
	float			GetMaximum(int prop) const;		//!< Return minimum value of property.
	float			GetValue(int prop) const;		//!< Return current value of property.
	const TCHAR*	GetName(int prop) const;		//!< Return name of property.
	bool			IsEnabled(int prop) const;		//!< Return  true if property is enabled for display.
	virtual void	AddValue(int prop, float val);	//!< Add to current value of property.
	virtual void	SetValue(int prop, float val);	//!< Set current value of property.
	virtual void	EnableProp(int prop);			//!< Mark property as enabled for display.
	virtual void	DisableProp(int prop);			//!< Disable display of given property.

	//! Add new property with the given name.
	virtual int		AddProperty(const TCHAR* name, int prop = -1);

	//! Print statistics properties and values.
	virtual DebugOut&		Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	virtual bool	Do(Messenger&, int);
	virtual void	Update();
	virtual void	UpdateLog() const;
	virtual bool	OpenLog(const TCHAR* logfile, const TCHAR* prefix);
	virtual void	CloseLog();
	virtual void	AddLog(const TCHAR* str) const;
	virtual void	Gather(float);

protected:
	int32			m_Frame;			// current frame number
	float			m_StartTime;		// start time of app
	float			m_LastTime;			// last frame time sampled
	const TCHAR*	m_Prefix;
	FILE*			m_LogFile;
	Core::String	m_StatusString;
	StatProp		m_Stats[STAT_MaxProp];
};

inline const TCHAR* FrameStats::GetString() const
	{ return m_StatusString; }

} // end Vixen