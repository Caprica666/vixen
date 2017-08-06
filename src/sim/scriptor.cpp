#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Scriptor, Sequencer, VX_Scriptor);

static const TCHAR* opnames[] = {
	TEXT("Exec"), TEXT("LoadScript"), TEXT("SetDirectory"),
};

float		Scriptor::s_AnimScale(1.0f);

const TCHAR** Scriptor::DoNames = opnames;

Scriptor::Scriptor() : Sequencer()
{
	m_Parser.Scriptor = this;
	m_Parser.Debug = Debug;
}

SharedObj*	Scriptor::Find(const TCHAR* name)
{
	return ScriptParser::FindObj(name);
}

/*!
 * @fn void Scriptor::SetDirectory(const TCHAR* dir)
 * @param dir script directory
 *
 * Designates the directory to load scripts from. This string is prepended
 * to script names which are not fully qualified.
 *
 * @see Scriptor::LoadScript
 */
void Scriptor::SetDirectory(const TCHAR* dir)
{
	VX_STREAM_BEGIN(stream)
		*stream << OP(VX_Scriptor, SCRIPT_SetDirectory) << this << dir;
	VX_STREAM_END( )

	m_Directory = dir;
} 

/*!
 * @fn void Scriptor::SetAnimationScale(float f)
 * @param f scale factor for animation positions
 *
 * Sets a scale factor for positions within animations.
 * This scale factor is applied when the animation is loaded from a file
 * (both .VIX and .BVH formats).
 *
 * For example, you can set it to 0.01 to scale centimeters to millimeters.
 * The scale factor does not affect rotations, which are unit quaternions.
 *
 * @see Interpolator::ScaleKeys
 */
void Scriptor::SetAnimationScale(float f)
{
	if (f > 0)
		s_AnimScale = f;
} 

/*!
 * @fn void Scriptor::Exec(const TCHAR* s)
 * @param s	string containing script to execute
 *			NULL to execute current script
 *
 * Executes the input string as a script. If run from
 * the display thread, the script is run immediately
 * and this routine returns the error code.
 * Otherwise, the script is posted to the messenger and
 * is executed at the start of the next display frame.
 * If this scriptor is set to emit events, the Event::EXEC event is
 * logged to indicate script completion.
 *
 * @return 0 if execution successful, else error code < 0
 *
 * @see LoadEvent SharedObj::SetFlags Scriptor;:ExecAsync Scriptor::GetError
 */
void Scriptor::Exec(const TCHAR* s)
{
	VX_STREAM_ASYNC_BEGIN(stream)
		*stream << OP(VX_Scriptor, SCRIPT_Exec) << this << s;
	VX_STREAM_ASYNC_END( )

	int err = m_Parser.Exec(s);
	if (IsSet(SharedObj::DOEVENTS))
	{
		LoadTextEvent* e = new LoadTextEvent(Event::EXEC);
		e->FileName = m_FileName;
		e->Sender = this;
		if (err)
			e->Text = TEXT("error");
		e->Log();
	}
}


/****
 *
 * OnEvent responds to the event generated when a script finishes loading
 * and to the events it is handling in the script
 *
 ****/
bool Scriptor::OnEvent(Event* event)
{
	LoadTextEvent*	fe = (LoadTextEvent*) event;
	SharedObj*	sender = fe->Sender;

	switch (fe->Code)
	{
		case Event::LOAD_TEXT:
		if (sender != this)
			return false;
		Exec(fe->Text);
	}
	return true;
}

int Scriptor::Save(Messenger& s, int opts) const
{
	int32 h = Engine::Save(s, opts);
	if ((h <= 0) || m_Directory.IsEmpty())
		return h;
	s << OP(VX_Scriptor, SCRIPT_SetDirectory) << h << m_Directory;
	return h;
}


/****
 *
 * class Scriptor override for SharedObj::Do
 *
 ****/
bool Scriptor::Do(Messenger& s, int op)
{
	TCHAR	name[VX_MaxString];

	switch (op)
	{
		case SCRIPT_LoadScript:
		s >> name;
		LoadScript(name);
		break;

		case SCRIPT_Exec:
		s >> name;
		Exec(name);
		break;

		case SCRIPT_SetDirectory:
		s >> name;
		SetDirectory(name);
		break;

		default:
		return Sequencer::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Scriptor::DoNames[op - SCRIPT_Exec]
					   << " " << this);
#endif
	return true;
}

/*! *
 * @fn bool Scriptor::WhenEvent(int code, SharedObj* sender, const TCHAR* s)
 * @param s			string containing script to execute
 * @param code		event code to check
 * @param sender	sending object to check
 *
 * Attaches the input script to the sender object and
 * executes it whenever that object sends the event code
 *
 * @return  true if successful, else  false
 *
 * @see Scriptor::Exec Scriptor::SetScript Scriptor::Load
 */
bool Scriptor::WhenEvent(int code, SharedObj* sender, const TCHAR* s)
{
	return m_Parser.WhenEvent(code, sender, s);
} 

bool	Scriptor::Copy(const SharedObj* animobj)
{
	const Scriptor* src = (const Scriptor*) animobj;
	
	ObjectLock dlock(this);
	ObjectLock slock(src);
	if (!Sequencer::Copy(animobj))
		return false;
	if (animobj->IsClass(VX_Scriptor))
	{
		m_Parser = src->m_Parser;
		m_Directory = src->m_Directory;
		m_FileName = src->m_FileName;
	}
	return true;

}

/*!
 * @fn void Scriptor::LoadScript(const TCHAR* file)
 * @param file	name of script file to load (usually scripts are in .SCP files)
 *
 * Loads a text file containing a script and executes it.
 * The commands in the script will be executed by the simulation thread
 * where it is safe to modify the hierarchy without stopping scene traversal.
 *
 * @see Scriptor::SetTarget
 */
void Scriptor::LoadScript(const TCHAR* file)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Scriptor, SCRIPT_LoadScript) << this << file;
	VX_STREAM_END(  )

	TCHAR			name[VX_MaxPath];
	TCHAR			dirbuf[VX_MaxPath];
	const TCHAR*	dir = dirbuf;

	Core::Stream::ParseDirectory(file, name, dirbuf);
	GetMessenger()->Observe(this, Event::LOAD_TEXT, this);
	if ((*dir == 0) && (dir = m_Directory) && *dir)
	{
		STRCPY(name, dir);
		STRCAT(name, TEXT("/"));
		STRCAT(name, file);
	}
	else
		STRCPY(name, file);
	m_FileName = file;
	World3D::Get()->LoadAsync(name, this);
}

}	// end Vixen