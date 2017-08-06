#pragma once

namespace Vixen {

inline Messenger* GetMessenger()
{	return BufMessenger::Get(); }

inline	const TCHAR*	Messenger::GetPath(const TCHAR* name, TCHAR* buf, int buflen)
{
	if (m_InStream.IsNull())
		return NULL;
	return m_InStream->GetPath(name, buf, buflen);
}

inline int Messenger::GetConnectID() const
	{ return m_ConnectID; }

inline Core::Stream* Messenger::GetInStream() const
	{ return m_InStream; }

inline Core::Stream* Messenger::GetOutStream() const
	{ return m_OutStream; }

inline void Messenger::SetInStream(Core::Stream* stream)
	{ m_InStream = stream; }

inline void Messenger::SetOutStream(Core::Stream* stream)
{
	m_OutStream = stream;
	SendUpdates = true;
}

inline NameTable* Messenger::GetNameDict() const
	{ return m_Names; }

inline Core::Buffer*	BufMessenger::GetReadBuf() const
{
	return m_ReadBuf;
}

inline Core::Buffer*	BufMessenger::GetWriteBuf()
{
	int	logtype = t_WriteLogType;
	return t_WriteBuf[logtype];
}

inline int		BufMessenger::GetReadLog() const
{
	return t_ReadLogType;
}

inline int		BufMessenger::GetWriteLog() const
{
	return t_WriteLogType;
}

inline void BufMessenger::SelectReadLog(int logtype)
{
	t_ReadLogType = logtype;
}

inline void BufMessenger::SelectWriteLog(int logtype)
{
	t_WriteLogType = logtype;
}

/****
 *
 * class MessLock is used to prevent unwanted transaction logging.
 * After a transaction has been logged, MessLock clears the DoLog
 * flag for the stream. The MessLock destructor clears the DoLog
 * flag so transaction logging is resumed after return
 *
 ****/
class MessLock
{
public:
	MessLock(Messenger* m, const SharedObj* obj) { Mess = m; Object = (SharedObj*) obj; }
	~MessLock();
	bool	CanLog(int logtype = Messenger::MESS_UpdateLog);
	void	EndLog();
	bool	NoLog;
	SharedObj*	Object;
	Messenger* Mess;
};

/****
 *
 * Determine if this transaction can be logged. If there is no stream
 * or logging is disabled, the transaction is not logged even if
 * the object is global and the stream is available.
 *
 ****/
inline bool MessLock::CanLog(int logtype)
{
	if (Mess == NULL)
		return false;
	NoLog = Mess->t_NoLog;
	if (NoLog)
		return false;
	Mess->t_NoLog = true;
	return Mess->BeginOp(logtype, Object);
}

/****
 *
 * DoLog is only set if this transaction was logged. In this case, we
 * end the transaction and enable logging again. Otherwise, we indicate
 * the object has been changed
 *
 ****/
inline void MessLock::EndLog()
{
	if (Mess && !NoLog)
	{
		Mess->EndOp(); 
		Mess->t_NoLog = NoLog;
	}
}

inline MessLock::~MessLock()
{
	if (Object)
		Object->Unlock();
}

/*!
 * @def VX_STREAM_BEGIN(_MESS)
 *
 * Starts an output stream update section. This macro
 * encapsulates the glue code to direct an update operation
 * to an output stream using the main messenger.
 *
 * @see VX_STREAM_END Messenger SharedObj::IsGlobal
 */
#ifndef VX_STREAM_BEGIN
#ifdef VX_ENABLE_DISTRIB
#define	VX_STREAM_BEGIN(_MESS)				\
	Messenger* _MESS = GetMessenger();		\
	MessLock _slock(_MESS, this);			\
	if ((SharedObj::IsGlobal() != 0) && _slock.CanLog()) {

#else
#define	VX_STREAM_BEGIN(_MESS)					\
	ObjectLock _WRITELOCK(this);				\
	if (0)										\
	{											\
		Messenger* _MESS = GetMessenger();		\


#endif
#endif

/*!
 * @def VX_STREAM_END()
 *
 * Ends an output stream update section. This macro
 * disables transaction logging for the code that
 * updates the Vixen object. It is enabled upon return.
 *
 *	@see VX_STREAM_BEGIN Messenger SharedObj::IsGlobal
 */
#pragma warning(disable:4003)

#ifndef VX_STREAM_END
#ifdef VX_ENABLE_DISTRIB
#define VX_STREAM_END( x ) _slock.EndLog(); } Lock();

#else
#define VX_STREAM_END( x )  } 
#endif
#endif

/*!
 * @def VX_STREAM_ASYNC_BEGIN(_MESS)
 *
 * Starts an asynchronous command. If issued in the display thread,
 * the command executes and logs to the update log.
 * If issued in a foreign thread, the command is logged to the
 * local log and NOT executed. 
 *
 * @see VX_STREAM_BEGIN VX_STREAM_ASYNC_END Messenger SharedObj::IsGlobal
 */
#ifndef VX_STREAM_ASYNC_BEGIN
#define	VX_STREAM_ASYNC_BEGIN(_MESS)		\
	Messenger* _MESS = GetMessenger();	\
	MessLock _slock(_MESS, this);			\
	int log = (Scene::GetTLS()->ThreadType & SCENE_DisplayThread) ? Messenger::MESS_UpdateLog : Messenger::MESS_LocalLog; \
	if (((SharedObj::IsGlobal() != 0) || (log == Messenger::MESS_LocalLog)) && _slock.CanLog(log)) {
#endif

/*!
 * @def VX_STREAM_ASYNC_END()
 *
 * Ends an asynchronous command and returns.
 *
 *	@see VX_STREAM_BEGIN Messenger SharedObj::IsGlobal
 */
#ifndef VX_STREAM_ASYNC_END
#define VX_STREAM_ASYNC_END( x )	 		\
	_slock.EndLog();						\
	if (log == Messenger::MESS_LocalLog)	\
		{ _slock.Object = NULL; return; }	\
	}  SharedObj::Lock();

#endif

} // end Vixen