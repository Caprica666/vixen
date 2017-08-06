#include "vixen.h"

namespace Vixen {
VX_IMPLEMENT_CLASSID(Switcher, Engine, VX_Switcher);

static const TCHAR* opnames[] =
{	TEXT("SetSize"), TEXT("SetAt"), TEXT("Add"), TEXT("SetTime"), TEXT("UNUSED"), TEXT("SetTimeSpan") };

const TCHAR** Switcher::DoNames = opnames;

Switcher::Switcher() : Engine()
{
	m_Index = -1;
	m_Lastt = 0;
}

/*!
 * @fn void Switcher::SetSize(int n)
 *
 * Sets the maximum number of times hold.
 * This is really just a hint to the system because the
 * internal arrays will dynamically grow as new times are appended.
 *
 * @see Switcher::Add
 */
void Switcher::SetSize(int n)
{
	m_Time.SetMaxSize(n);
}

/*!
 * @fn bool Switcher::Add(float time)
 * @param time Time for next index switch.
 *
 * Appends a new time onto the end of the time vector.
 * Times should be in ascending order.
 *
 */
bool Switcher::Add(float time)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Switcher, SWITCHER_Add) << this << time;
	VX_STREAM_END( )

	m_Time.Append(time);
	return true;
}

/*!
 * @fn void Switcher::SetAt(int n, float time)
 * @param n		0 based index of time vector entry to update.
 * @param time	new time
 *
 * Each entry in the time vector designates the time at which
 * the index makes its next switch. The times should be in
 * added in ascending order.
 *
 * @see Switcher::Add Switcher::SetTimeSpan
 */
void Switcher::SetAt(int n, float time)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Switcher, SWITCHER_SetAt) << this << int32(n) << time;
	VX_STREAM_END( )

	m_Time.SetAt(n, time);
}

/*!
 * @fn void Switcher::SetTimeSpan(float time)
 * @param time	new time span value
 *
 * If there is no time vector, a fixed interval expires
 * between index switches. The time span specifies the
 * number of seconds between index changes. It is ignored
 * if a time vector is present.
 *
 * @see Switcher::Add Switcher::SetAt
 */
void Switcher::SetTimeSpan(float time)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Switcher, SWITCHER_SetTimeSpan) << this << time;
	VX_STREAM_END(  )

	m_TimeSpan = time;
}


float Switcher::GetAt(int n) const
{
	if (n < 0 || n > m_Time.GetSize())
		return -1.0f;
	return m_Time[n];
}

/****
 *
 * Switcher override for Engine::Eval.
 * If the current time is greater than the next time
 * we should switch, change the switch index.
 * If we are past the end of the times array and the
 * engine is set to cycle, OnReset is called. Non cycling
 * engines call Stop (which calls the OnStop override).
 * OnChangeIndex is called if the switch index is updated.
 *
 ****/
bool Switcher::Eval(float t)
{	
	float	nexttime;
	int32	size = m_Time.GetSize();

	if (size == 0)						// no time array?
	   {
		if (m_TimeSpan <= 0)			// no timespan?
			return true;				// don't switch
		nexttime = m_Lastt + m_TimeSpan;
		ImageSwitch* target = (ImageSwitch*) (SharedObj*) GetTarget();
		if (target->IsClass(VX_ImageSwitch))
			size = target->GetSize();
	   }
	else								// use time array
	   {
		if (m_Index >= 0)				// not the first one?
			nexttime = m_Time[m_Index + 1];	// fetch next time
		else
			nexttime = m_Time[0];		// fetch first time
	   }
	if ((t - m_Lastt) < nexttime)		// time to get next one?
		return true;					// no, stay with this one
	m_Lastt = t;						// save input time
 	++m_Index;							// switch to next one
	if (m_Index >= size)				// beyond the last one?
	{
		if (!DoReset(t))
			Stop();						// just stop the engine
	}
	OnIndexChange(m_Index);				// index update
	return true;
}

void Switcher::OnIndexChange(int index)
{
	SharedObj* target = GetTarget();
	if (target == NULL)
		return;
	if (target->IsClass(VX_ModelSwitch))
		((ModelSwitch*) target)->SetIndex(index);
	else if (target->IsClass(VX_ImageSwitch))
		((ImageSwitch*) target)->SetIndex(index);
}

/****
 *
 * class Switcher override for Engine::OnReset
 * resets switch index 
 *
 ****/
bool Switcher::OnReset()
{
	m_Index = -1;
	m_Lastt = 0;
	return Engine::OnReset();
}

/****
 *
 * class Switcher override for Engine::OnStart
 * resets switch index 
 *
 ****/
bool Switcher::OnStart()
{
	m_Index = -1;
	m_Lastt = 0.0f;
	return Engine::OnStart();
}

/****
 *
 * class Switcher override for SharedObj::Copy
 *
 ****/
bool Switcher::Copy(const SharedObj* srcobj)
{
	ObjectLock dlock(this);
	ObjectLock slock(srcobj);
	if (!Engine::Copy(srcobj))
		return false;
	const Switcher* src = (const Switcher*) srcobj;
	if (src->IsClass(VX_Switcher))
		m_Time.Copy(&(src->m_Time));
	return true;
}

/****
 *
 * class Switcher override for SharedObj::Save
 *
 ****/
int Switcher::Save(Messenger& s, int opts) const
{
	int32 h = Engine::Save(s, opts);
	if (h <= 0)
		return h;
 	s << OP(VX_Switcher, SWITCHER_SetSize) << h << (int32) GetSize();
 	s << OP(VX_Switcher, SWITCHER_SetTimeSpan) << h << GetTimeSpan();
	for (int i = 0; i < GetSize(); ++i)
		s << OP(VX_Switcher, SWITCHER_Add) << h  << m_Time[i];
	return h;
}

DebugOut& Switcher::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Engine::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "\t<attr name='TimeSpan'>" << GetTimeSpan() << "</attr>");
	if (opts & PRINT_Data)
	{
		dbg << "<attr name='Times'>";
		for (int i = 0; i < GetSize(); ++i)
			dbg << m_Time[i] << " ";
		endl(dbg << "</attr>");
	}
	Engine::Print(dbg, opts & PRINT_Trailer);
	return dbg;
}

/****
 *
 * class Switcher override for SharedObj::Do
 *	SWITCH_SetSize		<int32>
 *	SWITCH_Add			<float>, <float>, ...
 *
 ****/
bool Switcher::Do(Messenger& s, int op)
{
	float	v;
	int32	i, n;

	switch (op)
	{
		case SWITCHER_SetSize:
		s >> n;
		SetSize(n);
		break;

		case SWITCHER_SetTimeSpan:
		s >> v;
		SetTimeSpan(v);
		break;

		case SWITCHER_Add:
		s >> v;
		Add(v);
		break;

		case SWITCHER_SetAt:
		s >> i >> v;
		SetAt(i, v);
		break;

		default:
		return Engine::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Switcher::DoNames[op - SWITCHER_SetSize]
					   << " " << this);
#endif
	return true;
}

}	// end Vixen