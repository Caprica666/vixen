/****
 *
 * inlines for Event
 *
 ****/
#pragma once

namespace Vixen {

/*!
 * @fn Event::Event(int code, int size, const SharedObj* sender)
 * @param code		event code, must be between 0 and EVENT_MaxCode
 * @param size		byte size of event data area, must be <= EVENT_MaxSize
 * @param sender	object sending the event
 *
 * @see Event::operator= Event::Event
 */
inline Event::Event(int code, int size, const SharedObj* sender) : List()
{
	VX_ASSERT(code < MAX_CODE);
	VX_ASSERT(size <= MAX_BYTES);
	Code = code;
	Size = size;
	Sender = sender;
}

/*!
 * @fn Event::Event(const Event& src)
 * @param src	event to copy from
 *
 * Clone an event by copying its code, size and data into this event.
 * All event subclasses should define Event::operator= to copy
 * the relevant event data. The default implementation only copies
 *  Code,  Size and  Sender. 
 *
 * @see Event::operator= Event::Event
 */
inline Event::Event(const Event& src)
{
	*this = src;
}

/*!
 * @fn Event& Event::Event(const Event& src)
 * @param src	event to copy from
 *
 * Copy the contents of one event into another.
 * All event subclasses should define Event::operator= to copy
 * the necessary event data. The default implementation only copies
 *  Code and  Size. Since it is possible that callers may
 * attempt to copy events of with differing codes, this function
 * should always check to event code of the source to confirm
 * the event data is appropriate for this subclass.
 *
 * @see Event::Event
 */
inline Event& Event::operator=(const Event& src)
{
	Code = src.Code;
	Size = src.Size;
	Sender = src.Sender;
	return *this;
}

/*!
 * @fn int Event::NewCode()
 *
 * Generates the next available event code. You can use this
 * function to generate event codes for your own event subclass.
 * An valid event code is between 1 and EVENT_MaxCode.
 *
 * @return event code or 0 if no more event codes can be generated
 *
 * @see Event::SetDataTypes Event::GetDataTypes
 */	
inline int	Event::NewCode()
{
	if (MaxCode >= MAX_CODE)
		return 0;
	return MaxCode += 1;
}
/*!
 * @fn const TCHAR* Event::GetDataTypes(int code)
 * @param code event code to get layout for
 *
 * Returns a string describing the format of the data words
 * associated for events with this code. Each word in the
 * event data is 32 bits and may be a number, string pointer or
 * object reference. Each character in the data types string
 * gives the type of a different data word. The types are
 * stored in the order the data words are stored. An event
 * may have up to 32 data words.
 *
 * <B>Event Data Types</B>
 *	@li \b F	32 bit float
 *	@li \b I	32 bit integer
 *	@li \b O	BaseObj*
 *	@li \b S	String
 *
 * @return	pointer to type string or NULL if no data
 *
 * @see Event::GetData Event::SetDataTypes
 */
inline const TCHAR* Event::GetDataTypes(int code)
{
	return TypeStrings[code];
}

/*!
 * @fn const TCHAR* Event::GetDataTypes() const
 *
 * Returns a string describing the format of the data words
 * of this event. Each word in the data area is 32 bits and
 * may be a number, string pointer or object reference.
 * Each character in the data types string
 * gives the type of a different data word. The types are
 * stored in the order the data words are stored.
 *
 * <B>Event Data Types</B>
 *	@li \b F	32 bit float
 *	@li \b I	32 bit integer
 *	@li \b O	BaseObj*
 *	@li \b S	String
 *
 * @return	pointer to type string or NULL if no data
 *
 * @see Event::GetData Event::SetDataTypes
 */
inline const TCHAR* Event::GetDataTypes() const
{
	return TypeStrings[Code];
}

/*!
 * @fn void Event::SetDataTypes(int code, const TCHAR* typestr)
 * @param code event code to set types for
 * @param typestr string describing types in event data word
 *
 * the input string describes the format of the data words of this event.
 * Each word in the data area is 32 bits and
 * may be a number, string pointer or object reference.
 * Each character in the data types string
 * gives the type of a different data word. The types are
 * stored in the order the data words are stored.
 *
 * This function is intended to be used only at initialization. It is
 * an error to set the data types for an event code more than once.
 *
 * <B>Event Data Types</B>
 *	@li \b F	32 bit float
 *	@li \b I	32 bit integer
 *	@li \b O	SharedObj*
 *	@li \b S	String
 *
 * @return	pointer to type string or NULL if no data
 *
 * @see Event::GetData Event::SetDataTypes Event::NewCode
 */
inline void Event::SetDataTypes(int code, const TCHAR* typestr)
{
	if ((code < 0) || (code > MAX_CODE))
		return;
	TypeStrings[code] = typestr;
}

/*!
 * @fn const TCHAR* Event::GetName(int code)
 *
 * Returns a string with the name of the event. Event names
 * are associated with an event code, not with an event subclass.
 * This permits a single base class to be used for a category
 * of events with different codes, types and names.
 *
 * @return pointer to type string or NULL if none
 *
 * @see Event::GetData Event::SetName Event::GetDataTypes
 */
inline const TCHAR* Event::GetName() const
{
	return Names[Code];
}

/*!
 * @fn const TCHAR* Event::GetName() const
 *
 * Returns a string with the name of this event. Event names
 * are associated with an event code, not with an event subclass.
 * This permits a single base class to be used for a category
 * of events with different codes, types and names.
 *
 * @return pointer to type string or NULL if none
 *
 * @see Event::GetData Event::SetName Event::GetDataTypes
 */
inline const TCHAR* Event::GetName(int code)
{
	if ((code < 0) || (code > MAX_CODE))
		return NULL;
	return Names[code];
}

/*!
 * @fn void Event::SetName(int code, const TCHAR* name)
 * @param code	event code to set name for
 * @param name	string name of event
 *
 * Establishes the string name for this event code. Event names
 * are associated with an event code, not with an event subclass.
 * This permits a single base class to be used for a category
 * of events with different codes, types and names.
 *
 * @see Event::GetName Event::SetDataTypes
 */
inline void Event::SetName(int code, const TCHAR* name)
{
	if ((code < 0) || (code > MAX_CODE))
		return;
	Names[code] = name;
}


} // end Vixen