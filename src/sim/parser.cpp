#include "vixen.h"
#include "vxutil.h"

namespace Vixen {

inline const TCHAR* SKIPBLANKS(const TCHAR* s)
{
	if (s == NULL)
		return s;
	while ((*s == TEXT(' ')) || (*s == TEXT('\t')))
	{
		if (*s == 0)
			return s;
		++s;
	}
	return s;
}

inline bool ENDOFLINE(const TCHAR* s)
{
	switch (*s)
	{
		case 0:
		case TEXT('\r'):
		case TEXT('\n'):
		case TEXT(';'):
		case TEXT('}'):
		return true;

		default:
		return false;
	}
}

inline bool NOTBLANK(const TCHAR* s)
{
	return !ENDOFLINE(s) && !isspace(*s);
}

inline const TCHAR* NEXTLINE(const TCHAR* s)
{
	while (*s)
		switch (*s++)
		{
			case TEXT('\r'):
			case TEXT('\n'):
			case TEXT(';'):
			case TEXT(' '):
			case TEXT('\t'):
			break;

			default:
			return s - 1;
		}
	return s;
}

ScriptParser::ScriptParser()
{
	Scriptor = NULL;	
	Error = 0;
}

/****
 *
 * OnEvent responds to the event generated when a script finishes loading
 * and to the events it is handling in the script
 *
 ****/
bool ScriptParser::OnEvent(Event* e)
{
	SharedObj*		target = NULL;
	const TCHAR*	script = NULL;
	Event*			event = (Event*) e;
	SharedObj*		sender = event->Sender;

	if (event->Code == Event::KEY)			// process keypress?
	{
		script = GetName(SCRIPT_Key + ((KeyEvent*) event)->KeyCode);
		if (script)
			Exec(script);
		return true;
	}
	if (sender->IsClass(VX_Animator))
		script = sender->GetName(SCRIPT_Key + event->Code);
	if (sender->IsClass(VX_Engine))
		target = ((Engine*) sender)->GetTarget();
	if ((script == NULL) && target)
		script = target->GetName(SCRIPT_Key + event->Code);
	switch (event->Code)
	{
		case Event::ENTER:
		case Event::LEAVE:
		if (script == NULL)
		{
			sender = ((TriggerEvent*) event)->Collider;
			if (sender)
				script = sender->GetName(SCRIPT_Key + event->Code);
		}
		if (script)
		{
			m_Event = *event;
			HandleHit((TriggerEvent*) event, script);
			m_Event.Code = Event::NONE;
			return true;
		}
		break;

		case Event::PICK:
		if ((sender == NULL) || !sender->IsClass(VX_Picker))
			return false;
		target = ((Picker*) sender)->GetNearest();
		if (target && (script = target->GetName(SCRIPT_Key + Event::PICK)))
		{
			m_Event = *event;
			HandlePick(event, script);
			m_Event.Code = Event::NONE;
			return true;
		}
		return false;
		
		default:
		if (script)
		{
			m_Event = *event;
			HandleEvent(event, script);
			m_Event.Code = Event::NONE;
			return true;
		}
		return false;
	}
	return true;
}


/*
 * @fn int ScriptParser::Exec(const TCHAR* s)
 * @param script	string containing script to execute
 *
 * Executes the input string as a script. If no string
 * is provided, the current script is re-executed.
 *
 * @return 0 if execution successful, else error code < 0
 */
int ScriptParser::Exec(const TCHAR* s)
{
	TCHAR buf[VX_MaxString];
	const TCHAR* e;
	const TCHAR* t;

	if ((s == NULL) || (*s == 0))
		return SCRIPT_ErrorEOF;
	Error = 0;
	VX_TRACE(Scriptor::Debug, (("Scriptor::Exec \n%s\n"), s));
	STRCPY(buf, s);	// copy script in case it is deleted during execution
	t = s = buf;
	while ((s = ExecLine(s)) &&
			(Error == 0) &&
			(*s != 0))
		t = s;
	switch (Error)
	{
		default:
		if (Error == 0)
			return 0;
		e = TEXT("parsing problem");
		break;

		case SCRIPT_ErrorBadCommand:	e = TEXT("illegal command"); break;
		case SCRIPT_ErrorBracket:		e = TEXT("mismatched brackets"); break;
		case SCRIPT_ErrorBadEvent:		e = TEXT("unknown event name"); break;
		case SCRIPT_ErrorObjName:		e = TEXT("illegal object name"); break;
		case SCRIPT_ErrorNotFound:		e = TEXT("object not found"); break;
		case SCRIPT_ErrorEOF:			e = TEXT("premature end of file"); break;
	}
	VX_ERROR(("Scriptor::Exec ERROR: %s\n%s\n", e, t), Error);		
	return Error;
} 

/*
 * @fn const TCHAR* ScriptParser::ExecLine(const TCHAR* s)
 * @param script	string containing script to execute
 *
 * Executes the first statement of the input string.
 * The <error> variable is set to an error code
 * when an error is encountered.
 *
 * @return -> after statement executed or NULL on error
 */
const TCHAR* ScriptParser::ExecLine(const TCHAR* s)
{
	TCHAR	name[VX_MaxPath];
	TCHAR*	d = name;
	const TCHAR*	t;

	s = SkipComments(s);
	if (s == NULL)
		return NULL;
	if (*s == TEXT('{'))							// do stuff in brackets
	{
		++s;
		while (*s != TEXT('}'))
			if (s = ExecLine(s))
				s = SkipComments(s);
			else
			{
				Error = SCRIPT_ErrorBracket;
				return NULL;
			}
		return s + 1;						// after matching bracket
	}
	if (*s == 0)
		return s;
	if (ENDOFLINE(s))						// end of line?
		return s + 1;
	t = ParseName(s, name);
	Core::String sname(name);
	if (t == 0)
	{
		Error = SCRIPT_ErrorBadCommand;
		return NULL;
	}
	if (sname.CompareNoCase(TEXT("load")) == 0)
		s = ParseLoad(t);
	else if (sname.CompareNoCase(TEXT("begin")) == 0)
		s = ParseBegin(t);
	else if (sname.CompareNoCase(TEXT("end")) == 0)
		s = ParseEnd(t);
	else if (sname.CompareNoCase(TEXT("kill")) == 0)
		s = ParseKill(t);
	else if (sname.CompareNoCase(TEXT("onevent")) == 0)
		s = ParseOnEvent(t);
	else if (sname.CompareNoCase(TEXT("onhit")) == 0)
		s = ParseOnHit(t);
	else if (sname.CompareNoCase(TEXT("onpick")) == 0)
		s = ParseOnPick(t);
	else if (sname.CompareNoCase(TEXT("onkey")) == 0)
		s = ParseOnKey(t);
	else if (sname.CompareNoCase(TEXT("enable")) == 0)
		s = ParseEnable(t);
	else if (sname.CompareNoCase(TEXT("disable")) == 0)
		s = ParseDisable(t);
	else if (sname.CompareNoCase(TEXT("distribute")) == 0)
		s = ParseDistribute(t);
	else if (sname.CompareNoCase(TEXT("camera")) == 0)
		s = ParseCamera(t);
	else 
		s = ParseLoad(s);
	if (s == NULL)
		return 0;
	s = SKIPBLANKS(s);
	if (*s == 0)
		return 0;
	return s;
} 

const TCHAR* ScriptParser::SkipComments(const TCHAR* s)
{
	s = NEXTLINE(s);
	while ((s[0] == TEXT('/')) && (s[1] == TEXT('/')))	// skip comments
	{
		s = SkipToEnd(s);
		s = NEXTLINE(s);
	}
	return s;
} 

/*
 * @fn bool ScriptParser::WhenEvent(int code, SharedObj* sender, const TCHAR* s)
 * @param s			string containing script to execute
 * @param code		event code to check
 * @param sender	sending object to check
 *
 * Attaches the input script to the <sender> object and
 * executes it whenever that object sends the event <code>
 *
 * @return  true if successful, else  false
 */
bool ScriptParser::WhenEvent(int code, SharedObj* sender, const TCHAR* s)
{
	TCHAR	name[VX_MaxString];

	if (sender == NULL)				// sender not found?
	{
		Error = SCRIPT_ErrorNotFound;
		return false;
	}
	STRNCPY(name, s, SkipToEnd(s) - s);
	GetMessenger()->Observe(this, code, sender);
	sender->SetName(name, SCRIPT_Key + code);
	return true;
} 

/*!
 * @fn bool ScriptParser::HandleEvent(Event* event, const TCHAR* s)
 * @param event	pick event to handle
 * @param s		string containing script to execute
 *
 * Determines whether the input script should be
 * executed for this event or not. If the event
 * code matches the first name in the script, 
 * the remainder of the script is executed.
 *
 * @return  true if event was handled,  false if not our event
 */
bool ScriptParser::HandleEvent(Event* event, const TCHAR* s)
{
	const TCHAR*	codestr = MakeEventString(event->Code);
	SharedObj*		sender = event->Sender;
	TCHAR			name[VX_MaxPath];
	const TCHAR*	oname = NULL;

	s = ParseName(s, name);		// get event name
	if ((s == NULL) || (codestr == NULL) ||
		(STRCASECMP(codestr, name) != 0))
		return false;
	if (*s == TEXT(','))				// skip comma after object name
		++s;
	Exec(s);					// execute rest of script
	return true;
}

/*
 * @fn bool ScriptParser::HandleHit(TriggerEvent* event, const TCHAR* s)
 * @param script	string containing script to execute
 *
 * Determines whether the input script should be
 * executed for this collision or not. The first
 * name in the script must match the name of the colliding
 * object in the event or it is ignored. If it matches,
 * the remainder of the script is executed.
 *
 * @return  true if event was handled,  false if not our event
 */
bool ScriptParser::HandleHit(TriggerEvent* event, const TCHAR* s)
{
	SharedObj*			sender = event->Sender;
	TCHAR			name[VX_MaxPath];
	const TCHAR*	oname = NULL;
	const TCHAR*	t = s;
	SharedObj*			collider = event->Collider;

	t = ParseName(s, name);		// get collider name
	if (s == NULL)
		return false;
	if ((STRCASECMP(name, TEXT("enter")) == 0) || (STRCASECMP(name, TEXT("leave")) == 0))
		return HandleEvent(event, s);
	if (STRCASECMP(name, TEXT("hit")) != 0)
		return false;
	s = t;
#if 0
	s = ParseName(t, name);		// get collider name
	if (s == NULL)
		return false;
	if (*name != TEXT('*'))
	{
		oname = sender->GetName();
		if (oname && STRCASECMP(name, oname))
		{
		if (collider == NULL)
			return false;
			oname = collider->GetName();
			if ((oname == NULL) || STRCASECMP(name, oname))
				return false;		// not our event
		}
	}
#endif
	if (*s == TEXT(','))				// skip comma after names
		++s;
	Exec(s);					// execute rest of script
	return true;
}

/*!
 * @fn bool ScriptParser::HandlePick(Event* event, const TCHAR* s)
 * @param event	pick event to handle
 * @param s		string containing script to execute
 *
 * Determines whether the input script should be
 * executed for this pick event or not. The first
 * name in the script must match the name of the 
 * object picked or it is ignored. If they match,
 * the remainder of the script is executed.
 *
 * @return  true if event was handled,  false if not our event
 */
bool ScriptParser::HandlePick(Event* event, const TCHAR* s)
{
	Picker*			sender = (Picker*) (SharedObj*) event->Sender;
	TCHAR			name[VX_MaxPath];
	SharedObj*		picked = sender->GetNearest();
	const TCHAR*	oname = NULL;

	s = ParseName(s, name);		// get object name
	if (s == NULL)
		return false;
	oname = picked->GetName();
	if ((*name != TEXT('*')) &&		// compare to specific object?
		STRCASECMP(name, oname))
		return false;
	if (*s == TEXT(','))				// skip comma after object name
		++s;
	Exec(s);					// execute rest of script
	return true;
}

/*
 * @fn const TCHAR* ScriptParser::ParseLoad(const TCHAR* s)
 * @param script	string containing script to execute
 *					NULL to execute current script
 *
 * Parses the "load" script command
 *
 *	load <filename> <engname> -cycle -kill -reverse -suspend -pingpong -target <targetname>
 *
 * @return -> after command parsed, NULL on error
 */
const TCHAR* ScriptParser::ParseLoad(const TCHAR* s)
{
	int				animopts = 0;
	int				engopts = 0;
	int				filetype;
	TCHAR			filename[VX_MaxPath];
	TCHAR			engbuf[VX_MaxPath];
	TCHAR			targname[VX_MaxPath];
	bool			suspend = false;
	Animator*		anim;
	SharedObj*		targobj = NULL;
	const TCHAR*	engname = NULL;

	*targname = 0;
	if ((s = ParseFile(s, filename, filetype)) == NULL)
		return NULL;
	if (isalpha(*s) && (s = ParseName(s, engbuf)))
		engname = engbuf;
	while (*s && (*s != TEXT('\n')))
	{
		s = SKIPBLANKS(s);
		if (s == NULL)
			break;
		if (*s != TEXT('-'))
			break;
		if (isdigit(s[1]))				// check for negative numbers
			break;
		++s;
		switch (*s++)
		{
			case TEXT('r'):					// -r or -reverse
			if (STRNCMP(s, TEXT("everse"), 6) == 0)
				s += 6;
			engopts |= Engine::REVERSE;
			break;

			case TEXT('p'):					// -p or -pingpong or -parent
			if (STRNCMP(s, TEXT("ingpong"), 7) == 0)
			{
				s += 7;
				engopts |= Engine::PING_PONG;
			}
			break;

			case TEXT('s'):					// -s or -suspend
			if (STRNCMP(s, TEXT("uspend"), 6) == 0)
				s += 6;
			suspend = true;
			break;

			case TEXT('c'):					// -c or -cycle
			if (STRNCMP(s, TEXT("ycle"), 4) == 0)
				s += 4;
			engopts |= Engine::CYCLE;	
			break;

			case TEXT('k'):					// -k or -kill
			if (STRNCMP(s, TEXT("ill"), 3) == 0)
				s += 3;
			animopts |= Animator::KILL_ON_STOP;
			break;

			case TEXT('t'):					// -t or -target
			if (STRNCMP(s, TEXT("arget"), 5) == 0)
				s += 5;
			if ((s = ParseName(s, targname)) == NULL)
				return NULL;
			targobj = FindObj(targname);
			break;

			case TEXT('m'):					// -m or -model
			if (STRNCMP(s, TEXT("odel"), 4) == 0)
				s += 5;
			if ((s = ParseName(s, targname)) == NULL)
				return NULL;
			targobj = FindModel(targname);
			filetype = VX_Model;
		}
	}
	if (!suspend)
		animopts |= Animator::AUTO_PLAY;
	switch (filetype)
	{
		case VX_Scriptor:
		Scriptor->Load(filename);
		break;

		case VX_Model:
		if (targobj && targobj->IsClass(VX_Model))		// target is a model?
		{
			ExtModel* xref = new ExtModel;
			xref->SetOptions(ExtModel::REMOVE | ExtModel::AUTOLOAD);
			((Model*) targobj)->Append(xref);
			xref->Load(filename);
			VX_TRACE(Debug, ("Scriptor::Exec load xref %s\n", filename));
			return s;
		}
		break;

		case 0:
		case VX_Engine:
		anim = Scriptor->Load(filename, engname, animopts, targobj);
		if (anim)
		{
			anim->Enable(engopts);
			GetMessenger()->Define(anim->GetName(), anim);
		}
		break;

#ifdef VX_SOUND
		case VX_SoundPlayer:
		Scriptor->LoadSound(filename, targobj, animopts);
		break;
#endif

#ifdef VX_VIDEO
		case VX_VideoImage:
		Scriptor->LoadVideo(filename, targobj, animopts);
		break;
#endif
	}
	VX_TRACE(Debug, ("Scriptor::Exec load %s\n", filename));
	return s;
} 

/*
 * @fn const TCHAR* ScriptParser::ParseBegin(const TCHAR* s)
 * @param script	string containing script to execute
 *
 * Parses the "begin" script command
 *
 *	begin <animname> <duration> -cycle -reverse -blendto -blend -relative
		-follow <follow> -with <with> -target <target> -start F -duration F -offset F -speed F
 *
 * @return -> after command parsed, NULL on error
 */
const TCHAR* ScriptParser::ParseBegin(const TCHAR* s)
{
	int				animopts = 0;
	int				engopts = 0;
	TCHAR			animname[VX_MaxPath];
	TCHAR			target[VX_MaxPath];
	TCHAR			follow[VX_MaxPath];
	Animator*		anim;
	SharedObj*		targobj = NULL;
	float			dur = 0.0f;
	float			start = 0.0f;
	float			speed = 1.0f;
	float			ofs = 0.0f;
	float			blendtime = 0;
	int32			blendopt = 0;
	int32			playopts = 0;
	bool			dotime = false;
	TCHAR*			t;

	*target = 0;
	*follow = 0;
	if ((s = ParseName(s, animname)) == NULL)
		return NULL;
	while (!ENDOFLINE(s))
	{
		s = SKIPBLANKS(s);
		if (*s != TEXT('-'))
			break;
		++s;
		switch (*s++)
		{
			case TEXT('d'):					// -d or -duration
			if (STRNCMP(s, TEXT("uration"), 7) == 0)
				s += 7;
			dur = (float) STRTOD(s, &t);
			dotime = true;
			s = t;
			break;

			case TEXT('o'):					// -o or -offset
			if (STRNCMP(s, TEXT("ffset"), 5) == 0)
				s += 5;
			ofs = (float) STRTOD(s, &t);
			dotime = true;
			s = t;
			break;

			case TEXT('s'):					// -s or -speed, -start
			dotime = true;
			if (STRNCMP(s, TEXT("tart"), 4) == 0)
			{
				s += 4;
				start = (float) STRTOD(s, &t);
				s = t;
				break;
			}
			else if (STRNCMP(s, TEXT("peed"), 4) == 0)
				s += 4;
			speed = (float) STRTOD(s, &t);
			s = t;
			break;

			case TEXT('r'):					// -r or -reverse
			if (STRNCMP(s, TEXT("elative"), 7) == 0)
			{
				s += 7;
				playopts |= Animator::PLAY_RELATIVE;
				break;
			}
			if (STRNCMP(s, TEXT("everse"), 6) == 0)
				s += 6;
			engopts |= Engine::REVERSE;
			break;

			case TEXT('k'):					// -k or -kill
			if (STRNCMP(s, TEXT("ill"), 3) == 0)
				s += 3;
			animopts |= Animator::KILL_ON_STOP;
			break;

			case TEXT('c'):					// -c or -cycle
			if (STRNCMP(s, TEXT("ycle"), 4) == 0)
				s += 4;
			engopts |=  Engine::CYCLE;	
			break;

			case TEXT('f'):					// -f or -follow
			if (STRNCMP(s, TEXT("ollow"), 5) == 0)
				s += 5;
			s = SKIPBLANKS(s);
			if (*s == TEXT('$'))
			{
				follow[0] = TEXT('$');
				follow[1] = 0;
				++s;
			}
			else if ((s = ParseName(s, follow)) == NULL)
				return NULL;
			playopts |= Animator::PLAY_AFTER;
			break;

			case TEXT('w'):					// -w or -with
			if (STRNCMP(s, TEXT("ith"), 3) == 0)
				s += 3;
			s = SKIPBLANKS(s);
			if (*s == TEXT('$'))
			{
				follow[0] = TEXT('$');
				follow[1] = 0;
				++s;
			}
			else if ((s = ParseName(s, follow)) == NULL)
				return NULL;
			playopts |= Animator::PLAY_WITH;
			break;

			case TEXT('t'):					// -t or -target
			if (STRNCMP(s, TEXT("arget"), 5) == 0)
				s += 5;
			if ((s = ParseName(s, target)) == NULL)
				return NULL;
			targobj = FindObj(target);
			if (targobj == NULL)
				Error = SCRIPT_ErrorNotFound;
			break;

			case TEXT('b'):					// -b -blend or -blendto
			if (STRNCMP(s, TEXT("lendto"), 6) == 0)
			{
				s += 6;
				blendopt = Engine::BLEND_TO;
			}
			else if (STRNCMP(s, TEXT("lend"), 4) == 0)
			{
				s += 4;
				blendopt = Engine::BLEND_BETWEEN;
			} 
			blendtime = (float) STRTOD(s, &t);
			s = t;
		}
	}
	anim = Scriptor->MakeAnim(animname, targobj, true);
	if (anim == NULL)
		return s;
	if (dotime)
	{
		anim->SetStartTime(start);
		anim->SetTimeOfs(ofs);
		anim->SetDuration(dur);
		anim->SetSpeed(speed);
	}
	anim->SetOptions(animopts | engopts);
	if (blendtime != 0)
		anim->Blend(blendtime, blendopt);
	Scriptor->Begin(animname, follow, targobj, playopts);
	VX_TRACE(Debug, ("Scriptor::Exec begin %s at %f offset %f\n", animname, anim->GetStartTime(), anim->GetTimeOfs()));
	return s;
} 

/*
 * @fn const TCHAR* ScriptParser::ParseEnd(const TCHAR* s)
 * @param script	string containing script to execute
 *
 * Parses the "end" script command
 *
 *	end <animname>
 *
 * @return -> after command parsed, NULL on error
 */
const TCHAR* ScriptParser::ParseEnd(const TCHAR* s)
{
	int				animopts = 0;
	TCHAR			animname[VX_MaxPath];

	if ((s = ParseName(s, animname)) == NULL)
		return NULL;
	Scriptor->End(animname);
	VX_TRACE(Debug, ("Scriptor::Exec end %s\n", animname));
	return s;
} 

/*
 * @fn const TCHAR* ScriptParser::ParseKill(const TCHAR* s)
 * @param script	string containing script to execute
 *
 * Parses the "kill" script command
 *
 *	kill <animname>
 *
 * @return -> after command parsed, NULL on error
 */
const TCHAR* ScriptParser::ParseKill(const TCHAR* s)
{
	int				animopts = 0;
	TCHAR			animname[VX_MaxPath];

	if ((s = ParseName(s, animname)) == NULL)
		return NULL;
	Scriptor->Kill(animname);
	VX_TRACE(Debug, ("Scriptor::Exec kill %s\n", animname));
	return s;
} 

/*****
 *
 * @fn const TCHAR* ScriptParser::ParseDistribute(const TCHAR* s)
 * @param script	string containing script to execute
 *
 * Parses the "distribute" script command
 *
 * @code
 *	distribute <objname> -g -n -s -d
 * @endcode
 *
 * @return -> after command parsed, NULL on error
 */
const TCHAR* ScriptParser::ParseDistribute(const TCHAR* s)
{
	int				opts = 0;
	TCHAR			objname[VX_MaxPath];
	SharedObj*			obj;
	uint32			classid = 0;

	if ((s = ParseName(s, objname)) == NULL)
		return NULL;
	if (STRCMP(objname, TEXT("class")) == 0)		// distribution for a class?
	{
		if ((s = ParseName(s, objname)) == NULL)
			return NULL;
		classid = SharedObj::FindClass(objname);
		if (classid <= 0)
		{
			Error = SCRIPT_ErrorNotFound;
			return NULL;
		}
	}
	while (!ENDOFLINE(s))
	{
		s = SKIPBLANKS(s);
		if (*s != TEXT('-'))
			break;
		++s;
		switch (*s++)
		{
			case TEXT('g'):					// -g or -global
			if (STRNCMP(s, TEXT("lobal"), 5) == 0)
				s += 5;
			opts |= GLOBAL | SHARED;
			break;

			case TEXT('n'):					// -n or -no
			if (s[1] == TEXT('o'))
				++s;
			opts = -1;
			break;

			case TEXT('s'):					// -s or -shared
			if (STRNCMP(s, TEXT("hared"), 5) == 0)
				s += 5;
			opts |= SHARED;
			break;

			case TEXT('d'):					// -d or -disable
			if (STRNCMP(s, TEXT("isable"), 6) == 0)
				s += 6;
			opts |= INACTIVE;
			break;
		}
	}
	if (classid)
	{
		if (classid == -1)
		  { VX_ERROR(("Scriptor::Exec ERROR: distribute class %s NOT FOUND\n", objname), s); }
		if (opts == -1)
			opts = 0;
		SharedObj::ShareClass(classid, opts);
		return s;
	}
	obj = FindObj(objname);				// distribution for object
	if (obj == NULL)					// object not found?
	   { VX_ERROR(("Scriptor::Exec ERROR: distribute %s NOT FOUND\n", objname), s); }
	GetMessenger()->Distribute(obj, opts);
	return s;
} 


/*
 * @fn const TCHAR* ScriptParser::ParseEnable(const TCHAR* s)
 * @param s	string containing script to execute
 *
 * Parses the "enable" script command which enables the named object.
 * If it is a model, it will become visible. If it is an engine,
 * it will become available for animation.
 *
 * @code
 *	enable <objname>
 * @endcode
 *
 * @return -> after command parsed, NULL on error
 */
const TCHAR* ScriptParser::ParseEnable(const TCHAR* s)
{
	TCHAR			name[VX_MaxPath];
	const SharedObj*	target;

	if ((s = ParseName(s, name)) == NULL)
		return NULL;
	target = FindObj(name);
	if (target == NULL)				// sender not found?
	{
		Error = SCRIPT_ErrorNotFound;
		return s;
	}
	VX_ASSERT(target->IsClass(VX_Group));
	((Group*) target)->SetActive(true);
	VX_TRACE(Debug, ("Scriptor::Exec enable %s\n", name));
	return s;
} 


/*
 * @fn const TCHAR* ScriptParser::ParseCamera(const TCHAR* s)
 * @param s	string containing script to execute
 *
 * Parses the "camera" script command which selects the given camera
 * as the current one for viewing
 *
 * @code
 *	camera <camname>
 * @endcode
 *
 * @return -> after command parsed, NULL on error
 */
const TCHAR* ScriptParser::ParseCamera(const TCHAR* s)
{
	TCHAR			name[VX_MaxPath];
	const SharedObj*	target;

	if ((s = ParseName(s, name)) == NULL)
		return NULL;
	target = FindObj(name);
	if (target == NULL)				// sender not found?
	{
		Error = SCRIPT_ErrorNotFound;
		return s;
	}
	VX_ASSERT(target->IsClass(VX_Camera));
	((Group*) target)->SetActive(true);
	GetMainScene()->SetCamera((Camera*) target);
	VX_TRACE(Debug, ("Scriptor::Exec camera %s\n", name));
	return s;
}

/*
 * @fn const TCHAR* ScriptParser::ParseDisable(const TCHAR* s)
 * @param s	string containing script to execute
 *
 * Parses the "disable" script command which disables the named object.
 * If it is a model, it will become invisible. If it is an engine,
 * it will stop executing.
 *
 * @code
 *	disable <objname>
 * @endcode
 *
 * @return -> after command parsed, NULL on error
 */
const TCHAR* ScriptParser::ParseDisable(const TCHAR* s)
{
	TCHAR			name[VX_MaxPath];
	const SharedObj*	target;

	if ((s = ParseName(s, name)) == NULL)
		return NULL;
	target = FindObj(name);
	if (target == NULL)				// sender not found?
	{
		Error = SCRIPT_ErrorNotFound;
		return s;
	}
	VX_ASSERT(target->IsClass(VX_Group));
	((Group*) target)->SetActive(false);
	VX_TRACE(Debug, ("Scriptor::Exec disable %s\n", name));
	return s;
} 

/*
 * @fn const TCHAR* ScriptParser::ParseOnEvent(const TCHAR* s)
 * @param script	string containing script to execute
 *
 * Parses the "onevent" script command
 *
 * @code
 *	onevent <event> <sendername>, { <statement> }
 * @endcode
 *
 * @return -> after command parsed, NULL on error
 */
const TCHAR* ScriptParser::ParseOnEvent(const TCHAR* s)
{
	TCHAR	buf[VX_MaxString];
	int32	code;
	size_t	n;
	const TCHAR*	script = s;

	s = ParseEventCode(s, code);	// parse event code name
	if ((s == NULL) || Error)
		return NULL;
	n = s - script;
	STRNCPY(buf, script, n);		// copy event code
	buf[n] = 0;
	GetMessenger()->Observe(this, code, NULL);
	VX_TRACE(Debug, ("Scriptor::Exec onevent %d\n", code));
	return MakeScript(s, buf, code);		// parse sender name, attach script
} 

/*
 * @fn const TCHAR* ScriptParser::ParseOnHit(const TCHAR* s)
 * @param script	string containing script to execute
 *
 * Parses the "onhit" script command
 *
 * @code
 *	onhit <name1> <name2>, { <statement> }
 * @endcode
 *
 * @return -> after command parsed, NULL on error
 */
const TCHAR* ScriptParser::ParseOnHit(const TCHAR* s)
{
	TCHAR			buf[VX_MaxString];
	const TCHAR*	script = s;

	STRCPY(buf, TEXT("hit "));
	GetMessenger()->Observe(this, Event::ENTER, NULL);
	return MakeScript(s, buf, Event::ENTER);		// parse sender name, attach script
} 

const TCHAR* ScriptParser::MakeScript(const TCHAR* s, TCHAR* buf, int code)
{
	TCHAR		name[VX_MaxPath];
	SharedObj*	sender;
	size_t		n;
	const TCHAR*	script = s;

	s = ParseName(s, name);			// parse sender name
	if (s == NULL)
		return NULL;
	sender = FindObj(name);
	script = s;
	s = SkipToEnd(s);
	n = s - script;
	STRNCAT(buf, script, n);
	VX_ASSERT(STRLEN(buf) < VX_MaxString);
	s = SkipToEnd(s);
	if (sender != NULL)
	{
		sender->SetFlags(SharedObj::DOEVENTS);	// enable sender to log events
		sender->SetName(buf, SCRIPT_Key + code);
		VX_TRACE(Debug > 1, ("Scriptor::Exec %s onevent = %s\n", sender->GetName(), buf));
	}
	else
		Error = SCRIPT_ErrorNotFound;
	return s;
} 

/*!
 * @fn const TCHAR* ScriptParser::ParseOnPick(const TCHAR* s)
 * @param s	string containing script to execute
 *
 * Parses the "onpick" script command
 *
 * @code
 *	onpick <objname> { <statement> }
 * @endcode
 *
 * @return -> after command parsed, NULL on error
 */
const TCHAR* ScriptParser::ParseOnPick(const TCHAR* s)
{
	TCHAR		name[VX_MaxString];
	SharedObj*	picked;
	size_t		n;
	const TCHAR*	script;

	script = s;
	s = ParseName(s, name);			// parse sender name
	if (s == NULL)
		return NULL;
	picked = FindModel(name);
	if (picked == NULL)				// picked object not found?
	{
		Error = SCRIPT_ErrorNotFound;
		return s;
	}
	s = SkipToEnd(s);
	n = s - script;
	STRNCPY(name, script, n);
	name[n] = 0;
	GetMessenger()->Observe(this, Event::PICK, NULL);
	VX_TRACE(Debug, ("Scriptor::Exec onpick %s\n", name));
	picked->SetName(name, SCRIPT_Key + Event::PICK);
	return s;
} 

/*
 * @fn const TCHAR* ScriptParser::ParseOnKey(const TCHAR* s)
 * @param script	string containing script to execute
 *
 * Parses the "onkey" script command
 *
 * @code
 *	onkey <key> { <statement> }
 * @endcode
 *
 * @return -> after command parsed, NULL on error
 */
const TCHAR* ScriptParser::ParseOnKey(const TCHAR* s)
{
	TCHAR		name[VX_MaxName];
	size_t		n;
	int32		keycode;
	const TCHAR*	script;

	s = ParseName(s, name);			// parse key
	if (s == NULL)
		return NULL;
	script = s;
	s = SkipToEnd(s);
	n = STRLEN(name);				// how long is keycode?
	if (n > 1)
		return NULL;
	keycode = *name;
	n = s - script;
	STRNCPY(name, script, n);
	name[n] = 0;
	GetMessenger()->Observe(this, Event::KEY, NULL);
	VX_TRACE(Debug, ("Scriptor::Exec onkey %s\n", name));
	SetName(name, SCRIPT_Key + keycode);
	return s;
} 

const TCHAR* ScriptParser::ParseFile(const TCHAR* s, TCHAR* namebuf, int& type)
{
	size_t		n;
	TCHAR		animname[VX_MaxPath];
	TCHAR*		d = animname;

/*
 * We should encounter either a filename or the name of an engine in the current
 * file. If it ends in ".vix", it is a filename
 */
	type = 0;
	d = animname;
	if (*s == 0)
		return NULL;
	while (NOTBLANK(s))
		*d++ = *s++;
	*d = 0;									// terminate filename string
	n = STRLEN(animname) - 4;				// find extension
	if (STRCASECMP(animname + n, TEXT(".vix")) == 0)
		type = VX_Engine;					// Vixen file
	if (STRCASECMP(animname + n, TEXT(".xml")) == 0)
		type = VX_Engine;					// Vixen file
	else if (STRCASECMP(animname + n, TEXT(".bvh")) == 0)
		type = VX_Engine;					// Vixen file
	else if (STRCASECMP(animname + n, TEXT(".hkt")) == 0)
		type = VX_Engine;					// Vixen file
	else if (STRCASECMP(animname + n, TEXT(".scp")) == 0)
		type = VX_Scriptor;				// SCP file
	else if (STRCASECMP(animname + n, TEXT(".wav")) == 0)
		type = VX_SoundPlayer;
	else if (STRCASECMP(animname + n, TEXT(".mp3")) == 0)
		type = VX_MediaSource;
	else if (STRCASECMP(animname + n, TEXT(".mpg")) == 0)
		type = VX_MediaSource;
	else if (STRCASECMP(animname + n, TEXT(".mov")) == 0)
		type = VX_MediaSource;
	else if (STRCASECMP(animname + n, TEXT(".avi")) == 0)
		type = VX_MediaSource;
/*
 * Parse file directory. If it is not a fully qualified path,
 * prepend the root directory if it has been specified
 */
	if (type > 0)
	{
		if ((Scriptor->GetDirectory() == NULL) ||	// given root directory?
			(*animname == TEXT('/')) ||				// full pathname?	
			(*animname == TEXT('\\')) ||
			(*animname == TEXT(':')))
			STRCPY(namebuf, animname);
		else										// prepend root directory
		{
			*namebuf = 0;
			STRCPY(namebuf, Scriptor->GetDirectory());
			d = namebuf + STRLEN(namebuf);
			if (d > namebuf)
			{
				*d++ = TEXT('/');
				*d = 0;
			}
			STRCAT(namebuf, animname);
		}
	}
	else
		STRCPY(namebuf, animname);
	return SKIPBLANKS(s);
} 

const TCHAR* ScriptParser::ParseName(const TCHAR* s, TCHAR* namebuf)
{
	TCHAR*	d = namebuf;

	if (*s == 0)
		return NULL;
	s = SKIPBLANKS(s);
	while (NOTBLANK(s) && (*s != TEXT(',')))
		*d++ = *s++;
	*d = 0;										// terminate filename string
	s = SKIPBLANKS(s);
	if (*namebuf == TEXT('$'))						// need to substitute?
		ExpandName(namebuf);
	return s;									// no, just return
} 

void ScriptParser::ExpandName(TCHAR* namebuf)
{
	Engine*		eng;
	SharedObj*		obj;
	Core::String	name(namebuf);

	if (name.CompareNoCase(TEXT("$target")) == 0) // return name of scriptor target
	{
		obj = Scriptor->GetTarget();
		if (obj && obj->GetName())
			STRCPY(namebuf, obj->GetName());
		return;
	}
	if (name.CompareNoCase(TEXT("$scriptor")) == 0)
	{										// return name of scriptor
		if (Scriptor->GetName())
		   STRCPY(namebuf, GetName());
		return;
	}
	if (name.CompareNoCase(TEXT("$camera")) == 0)
	{										// return name of camera
		Camera* cam = GetMainScene()->GetCamera();
		if (cam->GetName())
			STRCPY(namebuf, cam->GetName());
		return;
	}
	if (m_Event.Code == Event::NONE)
		return;
	eng = (Engine*) (SharedObj*) m_Event.Sender;
	if ((eng == NULL) || (eng->GetName() == NULL))
		return;
	if (name.CompareNoCase(TEXT("$sender")) == 0)
	{
		STRCPY(namebuf, eng->GetName());
		return;
	}
	if ((name.CompareNoCase(TEXT("$target")) == 0) && eng->IsClass(VX_Engine))
	{
		obj = eng->GetTarget();
		if (obj && obj->GetName())
			STRCPY(namebuf, obj->GetName());
	}
	if (name.CompareNoCase(TEXT("$collider")) == 0)
	{
		if ((m_Event.Code != Event::ENTER) && (m_Event.Code != Event::LEAVE))
			return;
		obj = ((TriggerEvent*) &m_Event)->Collider;
		if (obj && obj->GetName())
			STRCPY(namebuf, obj->GetName());
	}
}

const TCHAR*	ScriptParser::SkipToEnd(const TCHAR* s)
{
	while (!ENDOFLINE(s))
	{
		if (*s == TEXT('{'))
		{
			while (*s != TEXT('}'))
				s = SkipToEnd(s+ 1);
		}
		++s;
	}
	return s;
}

const TCHAR* ScriptParser::MakeEventString(int code)
{
	return Event::GetName(code);
}

const TCHAR* ScriptParser::ParseEventCode(const TCHAR* s, int32& code)
{
	TCHAR 		namebuf[20];
	const TCHAR* t = s;

	code = 0;
	s = ParseName(s, namebuf);
	if (s == 0)
	{
		Error = SCRIPT_ErrorBadEvent;
		return NULL;
	}
	Core::String name(namebuf);
	for (int i = 0; i < Event::MAX_CODE; ++i)
	{
		const TCHAR* evname = Event::GetName(i);
		if (evname == NULL)
			break;
		if (name.CompareNoCase(evname) == 0)
		{
			code = i;
			return s;
		}
	}
	Error = SCRIPT_ErrorBadEvent;
	return s;
}

SharedObj* ScriptParser::FindObj(const TCHAR* name)
{
	SharedObj* obj = GetMessenger()->Find(name);

	if (obj)
		return obj;
	obj = World3D::Get()->GetLoader()->Find(name);
	if (obj)
		return obj;
	obj = FindEngine(name, true);
	if (obj)
		return obj;
	return FindModel(name, true);
}

Engine* ScriptParser::FindEngine(const TCHAR* name, bool sceneonly)
{
	Engine*	eng;

	if (!sceneonly)
	{
		eng = (Engine*) GetMessenger()->Find(name);
		if (eng && eng->IsClass(VX_Engine))
			return eng;
		eng = (Engine*) World3D::Get()->GetLoader()->Find(name);
		if (eng && eng->IsClass(VX_Engine))
			return eng;
	}
//	Animator* anim = (Animator*) Scriptor->Find(name, Group::FIND_CHILD | Group::FIND_EXACT);
//	if (anim && anim->IsClass(VX_Animator))
//	{
//		eng = anim->GetRootEngine();
//		if (eng)
//			return eng;
//	}
	Scene*	scene = GetMainScene();
	eng = scene->GetEngines();
	if (eng)
	{
		eng = (Engine*) eng->Find(name, Group::FIND_DESCEND | Group::FIND_EXACT);
		if (eng)
			return eng;
	}
	return NULL;
}

Model* ScriptParser::FindModel(const TCHAR* name, bool sceneonly)
{
	Model*	mod;

	if (!sceneonly)
	{
		mod = (Model*) GetMessenger()->Find(name);
		if (mod && mod->IsClass(VX_Model))
			return mod;
		mod = (Model*) World3D::Get()->GetLoader()->Find(name);
		if (mod && mod->IsClass(VX_Model))
			return mod;
	}

	Scene*	scene = GetMainScene();
	mod = scene->GetModels();
	if (mod)
	{
		mod = (Model*) mod->Find(name, Group::FIND_DESCEND | Group::FIND_EXACT);
		if (mod)
			return mod;
	}
	return NULL;
}

}	// end Vixen