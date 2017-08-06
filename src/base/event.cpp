#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASS(Event, Core::List);

const TCHAR* Event::Names[Event::MAX_CODE];
const TCHAR* Event::TypeStrings[Event::MAX_CODE];
int	Event::MaxCode	= 0;

/*!
 * @fn bool Event::Log(int logtype = STREAM_EventLog)
 *
 * Logs the event to the messenger and free the input event.
 *
 * @return \b true if event was logged, else \b false
 *
 * @see Event::Save BaseObj::OnEvent
 */
bool	Event::Log(int logtype)
{
	Messenger*	mess = GetMessenger();
	if (mess == NULL)
	{
		delete this;
		return false;
	}
	if (Time == 0.0f)
		Time = World::Get()->GetTime();
	*mess << *this;
	delete this;
	return true;
}

/*!
 * @fn bool Event::Save(Messenger& stream) const
 * @param stream	stream to write to
 *
 * The base implementation writes the event data to the
 * stream based on the data types string for the event code.
 *
 * @see Event::SetDataTypes Event::Log Event::Parse World::MakeEvent
 */
bool Event::Save(Messenger& stream) const
{
	const TCHAR*	types = GetDataTypes();
	const char	*	data = (const char*) ((Event*) this)->GetData();
	SharedObj*		obj;
	Core::String*	sptr;
	const TCHAR*	s;

	stream << int32(Messenger::VIXEN_Event) << int32(Code) << int32(Size) << Sender << Time;
	if (types == NULL)
		return true;
	while (*types)				// scan characters of data type string
	{
		switch (*types)			// save data words based on type
		{
			case TEXT('I'): stream << *((int32*) data); data += sizeof(int32); break;
			case TEXT('F'): stream << *((float*) data); data += sizeof(float); break;
#ifdef _M_X64
			case TEXT('P'): stream << *((int64*) data); data += sizeof(int64); break;
#else
			case TEXT('P'): stream << *((int32*) data); data += sizeof(int32); break;
#endif
			case TEXT('O'): obj = *((ObjRef*) data); data += sizeof(ObjRef); stream << obj; break;
			case TEXT('S'):
			sptr = (Core::String*) data;
			data += sizeof(Core::String);
			if (sptr && sptr->GetLength())
				s = *sptr;
			else
				s = TEXT("");
			stream << s;
			break;

			default: return true;
		}
		++types;				// next data type
	}
	return true;				// indicate success
}

/*!
 * @fn bool Event::Parse(Messenger& stream)
 * @param stream	stream to read event from
 *
 * The base implementation Reads the event data from the
 * stream based on the data types string for the event code.
 * Upon entry, the Code of the event has already been read and stored in the event.
 *
 * @see Event::SetDataTypes Event::Log BaseObj::Seralize World::MakeEvent
 */
bool Event::Parse(Messenger& stream)
{
	const TCHAR* types = GetDataTypes();
	char*		data = (char*) GetData();
	int32		i;
	float		f;
	SharedObj*	obj;
	TCHAR		buf[VX_MaxString];

	stream >> i >> obj >> Time;
	Size = i;
	Sender = obj;
	if (types == NULL)
		return true;
	while (*types)				// scan characters of data type string
	{
		switch (*types)			// parse data words based on type
		{
			case TEXT('I'):
			stream >> i;
			*((int32*) data) = i;
			data += sizeof(int32);
			break;

			case TEXT('F'):
			stream >> f;
			*((float*) data) = f;
			data += sizeof(float);
			break;

			case TEXT('P'):
			{
#ifdef _M_X64
			int64 ptr;
			stream >> ptr;
			*((int64*) data) = ptr;
			data += sizeof(int64);
#else
			int32 ptr;
			stream >> ptr;
			*((int32*) data) = ptr;
			data += sizeof(int32);
#endif
			}
			break;

			case TEXT('O'):
			stream >> obj;
			*((void**) data) = NULL;
			*((ObjRef*) data) = obj;
			data += sizeof(ObjRef);
			break;

			case TEXT('S'):
			stream >> buf;
			new (data) Core::String(buf);
			data += sizeof(Core::String);
			break;

			default:
			break;
		}
		++types;					// next data type
	}
	return 1;						// indicate success
}

}	// end Vixen