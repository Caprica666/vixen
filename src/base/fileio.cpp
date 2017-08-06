#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(FileMessenger, Messenger, VX_FileMessenger);

/*****
 *
 * class FileMessenger is a subclass of stream that loads and stores
 * Vixen binary protocol from a disk file. File messengers do not share
 * their name dictionary with the global messenger. Each file messenger has
 * its own dictionary. When objects loaded from a file become part of
 * the scene, the file dictionary is merged with the global messenger
 * dictionary by the loader.
 *
 ****/
FileMessenger::FileMessenger(const TCHAR* filename) : Messenger()
{
	Version = MESS_CurrentVersion;
	if (filename)
		m_InStream = new Core::FileStream(filename);
}

int FileMessenger::Attach(const SharedObj* obj, int flags)
{
	if (obj == NULL)
		return 0;
	if (flags)
		((SharedObj*) obj)->SetFlags(flags);
	return m_Objs->AttachObj(obj);
}

SharedObj*	FileMessenger::Create(uint32 classid, int handle)
{
	SharedObj* obj = Messenger::Create(classid, handle);
	if (obj)
		obj->SetID(0);
	return obj;
}

Messenger& FileMessenger::OutObj(const SharedObj* obj)
{
	int32	handle = m_Objs->GetHandle(obj);
	Output(&handle, 1);
	return *this;
}

bool FileMessenger::Close()
{
	if (!m_OutStream.IsNull() && m_OutStream->IsOpen(OPEN_WRITE))
	{
		BeginOp(Messenger::MESS_UpdateLog);
		int32 v = VIXEN_End;				// write ISM_End at end
		m_OutStream->Write((const char*) &v, sizeof(int32));
		EndOp();
	}
	return Messenger::Close();
}

int	FileMessenger::CanSave(const SharedObj* obj, int opts)
{
	if (obj->IsSet(SAVED))
		return 0;
	int32 h = Attach(obj, SAVED);
	const TCHAR* name = obj->GetName();
	if (name)
		Define(name, obj);	
	if (h)
		*this << OP(uint16(obj->ClassID()), SharedObj::OBJ_Create) << h;
	return h;
}

bool FileMessenger::Open(const TCHAR* filename, int mode)
{
	int32 version[4] = { VIXEN_Version, MESS_CurrentVersion, (int32) VIXEN_VecSize, SysVecSize };

	Version = 0;
	if (mode & OPEN_WRITE)
	{
		if (!m_OutStream)
			m_OutStream = new Core::FileStream;
		Version = MESS_CurrentVersion;
	}
	if (!Messenger::Open(filename, mode))
		return false;
	m_Objs = new ObjMap;
	m_Names = new NameTable;
	if (mode & OPEN_WRITE)		// open for write
		Write((char*) &version, sizeof(version));				
	return true;
}

}	// end Vixen