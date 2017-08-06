#include "vixen.h"
#include "vxutil.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(FrameStats, Engine, VX_FrameStats);

static const TCHAR* opnames[] =
{
	TEXT("SetStatus"),
	TEXT("AddStatus"),
	TEXT("Enable"),
	TEXT("Disable")
};

const TCHAR** FrameStats::DoNames = opnames;

FrameStats::FrameStats() : Engine()
{
	m_Prefix = NULL;
	m_LogFile = NULL;
	m_Stats[STAT_FrameRate].Name = TEXT("Frames/Sec");
	m_Stats[STAT_VertsCulled].Name = TEXT("Culled Verts");
	m_Stats[STAT_VertsRendered].Name = TEXT("Rendered Verts");
	m_Stats[STAT_StateChanges].Name = TEXT("State Changes");
	m_Stats[STAT_PrimsRendered].Name = TEXT("Prims Rendered");
	m_Stats[STAT_ModelsRendered].Name = TEXT("Rendered Models");
	m_Stats[STAT_ModelsCulled].Name = TEXT("Culled Models");
	m_Stats[STAT_FrameTime].Name = TEXT("Frame Time");
	for (int i = 0; i < STAT_MaxProp; ++i)
		m_Stats[i].Reset();
	m_Stats[STAT_FrameRate].Enable = true;
	SetControl(Engine::CYCLE | Engine::CHILDREN_FIRST);
	m_Frame = 0;
	m_StartTime = 0;
}

FrameStats::~FrameStats() { CloseLog(); }

/*!
 *
 * @fn const TCHAR* FrameStats::GetName(int prop) const
 * @param prop property index (STAT_xxx)
 *
 * Returns the name string of a property given its index.
 *
 * @return name of property, NULL if property not defined or out of range
 *
 * @see FrameStats::AddProperty
 */
const TCHAR* FrameStats::GetName(int prop) const
{
	if ((prop < 0) || (prop >= STAT_MaxProp))
		return 0;
	return m_Stats[prop].Name;
}

/*!
 * @fn float FrameStats::GetAverage(int prop) const
 * @param prop property index (STAT_xxx)
 *
 * Gets the average of the given property over
 * the duratiion of the engine. Every time the
 * property is updated during the duration, the
 * new value is incorporated into the running average
 * for that property. If the duration is zero, averages
 * are not kept and GetAverage returns the current value
 * of the property.
 *
 * @return average value of this property
 *
 * @see FrameStats::GetStatus Engine::SetDuration FrameStats::GetMinimum FrameStats::GetMaximum
 */
float FrameStats::GetAverage(int prop) const
{
	if ((prop < 0) || (prop >= STAT_MaxProp) || (m_Stats[prop].Name == NULL))
		return 0;
	return m_Stats[prop].Average / m_Stats[prop].Samples;
}

/*!
 * @fn float FrameStats::GetMinimum(int prop) const
 * @param prop	property index (STAT_xxx)
 *
 * Gets the minimum value of a property given its index.
 * The minimum value is computed over the duration of
 * the run and is not reset when the engine cycles.
 *
 * @return minimum value of this property
 *
 * @see FrameStats::GetStatus FrameStats::GetMaximum FrameStats::GetAverage
 */
float FrameStats::GetMinimum(int prop) const
{
	if ((prop < 0) || (prop >= STAT_MaxProp) || (m_Stats[prop].Name == NULL))
		return 0;
	return m_Stats[prop].Minimum;
}

/*!
 * @fn float FrameStats::GetMaximum(int prop) const
 * @param prop	property index (STAT_xxx)
 *
 * Gets the maximum value of a property given its index.
 * The maximum value is computed over the duration of
 * the run and is not reset when the engine cycles.
 *
 * @return minimum value of this property
 *
 * @see FrameStats::GetStatus FrameStats::GetMinimum FrameStats::GetAverage
 */
float FrameStats::GetMaximum(int prop) const
{
	if ((prop < 0) || (prop >= STAT_MaxProp) || (m_Stats[prop].Name == NULL))
		return 0;
	return m_Stats[prop].Maximum;
}

/*!
 * @fn float FrameStats::GetValue(int prop) const
 * @param prop	property index (STAT_xxx)
 *
 * Gets the current value of the given property.
 *
 * @see FrameStats::GetAverage Engine::SetDuration
 */
float FrameStats::GetValue(int prop) const
{
	if ((prop < 0) || (prop >= STAT_MaxProp) || (m_Stats[prop].Name == NULL))
		return 0;
	return m_Stats[prop].Value;
}

/*!
 * @fn void FrameStats::SetValue(int prop, float val)
 * @param prop	property index (STAT_xxx)
 * @param val	new value of property
 *
 * Sets the next value of the given property.
 * When a property is updated, it contributes
 * to a running average and an overal minimum and maxmimum.
 *
 * @see FrameStats::GetAverage Engine::SetDuration
 */
void FrameStats::SetValue(int prop, float val)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_FrameStats, STAT_SetValue) << this << int32(prop) << val;
	VX_STREAM_END()

	if ((prop < 0) || (prop >= STAT_MaxProp) || (m_Stats[prop].Name == NULL))
		return;
	if (m_Duration == 0.0f)
	{
		m_Stats[prop].Samples = 1;
		m_Stats[prop].Average = m_Stats[prop].Value = val;
	}
	else
	{
		++(m_Stats[prop].Samples);
		m_Stats[prop].Average += val;
		m_Stats[prop].Value = val;
	}
	if (m_Stats[prop].Minimum == 0)
		m_Stats[prop].Minimum = m_Stats[prop].Value;
	else if (m_Stats[prop].Minimum > m_Stats[prop].Value)
		m_Stats[prop].Minimum = m_Stats[prop].Value;
	if (m_Stats[prop].Maximum < m_Stats[prop].Value)
		m_Stats[prop].Maximum = m_Stats[prop].Value;
	SetChanged(true);
}

/*!
 * @fn void FrameStats::AddValue(int prop, float val)
 * @param prop	property index (STAT_xxx)
 * @param val	amount to add to current value
 *
 * Adds to the current value of the given status property.
 * When a property is updated, it contributes
 * to a running average and an overal minimum and maxmimum.
 * Adding to a property does not constitute taking another sample.
 *
 * @see FrameStats::GetAverage Engine::SetDuration FrameStats::SetStatus
 */
void FrameStats::AddValue(int prop, float val)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_FrameStats, STAT_AddValue) <<  this << int32(prop) << val;
	VX_STREAM_END()

	if ((prop < 0) || (prop >= STAT_MaxProp) || (m_Stats[prop].Name == NULL))
		return;
	m_Stats[prop].Value += val;
	m_Stats[prop].Average += val;
	if (m_Stats[prop].Minimum == 0)
		m_Stats[prop].Minimum = m_Stats[prop].Value;
	else if (m_Stats[prop].Minimum > m_Stats[prop].Value)
		m_Stats[prop].Minimum = m_Stats[prop].Value;
	if (m_Stats[prop].Maximum < m_Stats[prop].Value)
		m_Stats[prop].Maximum = m_Stats[prop].Value;
	SetChanged(true);
}

/*!
 * @fn void FrameStats::EnableProp(int prop)
 * @param prop	property to update (STAT_xxx)
 *
 * Enables a particular property for display.
 * Does not affect gathering of statistics,
 * just which ones are accumulated
 * in the status string or sent to the text image target.
 *
 * @see FrameStats::GetAverage FrameStats::SetValue FrameStats::GetString
 */
void FrameStats::EnableProp(int prop)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_FrameStats, STAT_Enable) <<  this << int32(prop);
	VX_STREAM_END()

	if ((prop < 0) || (prop >= STAT_MaxProp) || (m_Stats[prop].Name == NULL))
		return;
	m_Stats[prop].Enable = true;
}

/*!
 * @fn void FrameStats::DisableProp(int prop)
 * @param prop	property to update (STAT_xxx)
  *
 * Disables a particular property for display.
 * Does not affect gathering of statistics,
 * just which ones are accumulated
 * in the status string or sent to the text image target.
 *
 * @see FrameStats::GetAverage FrameStats::SetValue FrameStats::GetString
 */
void FrameStats::DisableProp(int prop)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_FrameStats, STAT_Disable) <<  this << int32(prop);
	VX_STREAM_END()

	if ((prop < 0) || (prop >= STAT_MaxProp) || (m_Stats[prop].Name == NULL))
		return;
	m_Stats[prop].Enable = false;
}

bool FrameStats::IsEnabled(int prop) const
{
	if ((prop < 0) || (prop >= STAT_MaxProp) || (m_Stats[prop].Name == NULL))
		return false;
	return m_Stats[prop].Enable;
}

/*!
 * @fn int FrameStats::AddProperty(const TCHAR* name, int3prop)
 *
 * Adds a new  property to the frame statistics log.
 * The application is responsible for gathering the
 * information for this property and calling SetValue.
 * The new property will be averaged, 
 *can be enabled, disabled and printed.
 *
 * You can call AddProperty to change the name of
 * an existing property. If the property index
 * is within the range of the existing properties,
 * the new name string will be used.
 *
 * Args:
 * @param name	string name of  property
 * @param prop	desired ID of property. if less than zero,
 *				a new property ID is generated
 *
 * @return property ID of new property or negative on error 
 *
 * @see FrameStats::SetValue FrameStats::EnableProp
 */
int FrameStats::AddProperty(const TCHAR* name, int prop)
{
	if (prop < 0)
	{
		for (prop = 0; prop < STAT_MaxProp; ++prop)
			if (m_Stats[prop].Name == NULL)
			{
				m_Stats[prop].Name = name;
				return prop;
			}
		return 0;
	}
	if (prop >= STAT_MaxProp)
		return -1;
	if (m_Stats[prop].Name)		// already taken
	{
		m_Stats[prop].Name = name;	// change name anyway
		return -1;
	}
	m_Stats[prop].Name = name;		// new property name
	return prop;
}

/*!
 * @fn void FrameStats::Gather(float time)
 *
 * Gathers statistics from the 3D scene very frame. The following
 * status properties are updated by the default implementation:
 * @code
 *		STAT_FrameRate
 *		STAT_FrameTime
 *		STAT_RenderTime
 *		STAT_VertsCulled
 *		STAT_VertsTrnfrtrf
 *		STAT_PrimsRendered
 *		STAT_StateChanges
 *		STAT_ModelsRendered
 *		STAT_ModelsCulled
 * @endcode
 * Gather is called every frame by Eval. It can be overridden
 * to gather other statistics every frame.
 */
void FrameStats::Gather(float time)
{
	const SceneStats*	stats = GetMainScene()->GetStats();

	if ((stats == NULL) ||	(stats->StartTime == 0.0))
		return;
	if (m_StartTime == 0.0)
		m_StartTime = stats->StartTime;
	m_LastTime = stats->EndTime;
	SetValue(STAT_FrameTime, stats->EndTime - stats->StartTime);
	++m_Frame;
	SetValue(STAT_VertsCulled, float(stats->CulledVerts));
	SetValue(STAT_VertsRendered, float(stats->TotalVerts - stats->CulledVerts));
	SetValue(STAT_PrimsRendered, float(stats->PrimsRendered));
	SetValue(STAT_StateChanges, float(stats->RenderStateChanges));
	SetValue(STAT_ModelsRendered, float(stats->TotalModels - stats->CulledModels));
	SetValue(STAT_ModelsCulled, float(stats->CulledModels));
	UpdateLog();
}

void FrameStats::Update()
{
	char	line[256];
	char	buf[VX_MaxString];

	if (!HasChanged())
		return;
	*buf = 0;
	SetValue(STAT_FrameRate, 1 / GetAverage(STAT_FrameTime));
	for (int i = 0; i < STAT_MaxProp; ++i)
	{
		if (!m_Stats[i].Enable)
			continue;
		sprintf(line, "%6.2f %s\r\n", GetAverage(i), GetName(i));
		strcat(buf, line);
	}
	m_StatusString = buf;
	SharedObj* target = GetTarget();
	if (target)
	{
		if (target->IsClass(VX_TextGeometry))
			((TextGeometry*) target)->SetText(m_StatusString);
		else if (target->IsClass(VX_SimpleShape))
			((SimpleShape*) target)->SetText(m_StatusString);
	}
	SetChanged(false);
}

bool FrameStats::Eval(float time)
{
	Gather(time);
	if (m_Duration == 0)			// Update every frame
		Update();
	return true;
}

bool FrameStats::OnStart()
{
	for (int i = 0; i < STAT_MaxProp; ++i)
		m_Stats[i].Reset();
	m_Frame = 0;
	return true;
}

bool FrameStats::OnReset()
{
	if (m_Stats[STAT_FrameTime].Samples == 0)
		Gather(GetElapsed());
	if (HasChanged())
	{
		Update();
		UpdateLog();
	}
	for (int i = 0; i < STAT_MaxProp; ++i)
	{
		if (m_Stats[i].Name == NULL)
			continue;
		m_Stats[i].Average = 0;
		m_Stats[i].Samples = 0;
	}
	return true;
}

/*!
 * @fn DebugOut& FrameStats::Print(DebugOut& dbg, int opts) const
 *
 * Prints a summary of all status properties, their
 * average, minimum and maximum on the given debug
 * stream. DebugOut is a subclass of ostream.
 *
 * @see SharedObj::Print
 */
DebugOut&	FrameStats::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Engine::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "<attr name='stats'>");
	for (int i = 0; i < STAT_MaxProp; ++i)
		if ((m_Stats[i].Samples > 0) && m_Stats[i].Name)
			endl(dbg << " <" << m_Stats[i].Name
					 << " value='" << GetValue(i) << "'"
					 << " average='" << GetAverage(i) << "'"
					 << " minimum='" << GetMinimum(i) << "'"
					 << " maximum='" << GetMaximum(i) << "' />");
	endl(dbg << "</attr>");
	return Engine::Print(dbg, opts & PRINT_Trailer);
}

bool FrameStats::Do(Messenger& s, int op)
{
	float	v;
	int32	prop;

	switch (op)
	{
		case STAT_SetValue:
		s >> prop >> v;
		SetValue(prop, v);
		break;

		case STAT_AddValue:
		s >> prop >> v;
		AddValue(prop, v);
		break;
	
		case STAT_Enable:
		s >> prop;
		EnableProp(prop);
		break;
		
		case STAT_Disable:
		s >> prop;
		DisableProp(prop);
		break;

		default:
		return Engine::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << FrameStats::DoNames[op - STAT_SetValue]
					   << " " << this);
#endif
	return true;
}

bool FrameStats::OpenLog(const TCHAR* logfile, const TCHAR* prefix)
{
	m_LogFile = FOPEN(logfile, TEXT("w"));
	m_Prefix = prefix;
	AddLog("Frame:\tProperty\tAverage\tMinimum\tMaxmimum\n\n");
	return m_LogFile != 0;
}

void FrameStats::CloseLog()
{
	m_Stats[STAT_FrameTime].Average = m_LastTime - m_StartTime;
	m_Stats[STAT_FrameTime].Samples = m_Frame;
	UpdateLog();
	if (m_LogFile)
		fclose(m_LogFile);
	m_LogFile = NULL;
}

void FrameStats::AddLog(const TCHAR* string) const
{
	if (m_LogFile)
		fputs(string, m_LogFile);
}

void FrameStats::UpdateLog() const
{
	char	buf[256];

	for (int i = 0; i < STAT_MaxProp; ++i)
		if ((m_Stats[i].Samples > 0) && m_Stats[i].Name && m_Stats[i].Enable)
		{
			sprintf(buf, "%s %d:\t%s\t%f\t%f\t%f\n",
					m_Prefix, m_Frame, m_Stats[i].Name,
					GetAverage(i), GetMinimum(i), GetMaximum(i));
			AddLog(buf);
		}
}

}	// end Vixen