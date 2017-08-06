/*!
 * @file vxfile.h
 *
 * @brief Defines basic communication classes for I/O.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxmess.h vxstream.h
 */

#pragma once

namespace Vixen {

/*!
 * @class FileMessenger
 * @brief Load Vixen content from binary files or to save a Vixen scene in a file.
 *
 * This is the same format used by the <A HREF=exporter/maxartist.htm> MAX exporter</A>
 * when emitting files for use with the <A HREF=oview/viewer_help.htm> Vixen Viewer</A>.
 * Only Vixen objects which  implement the <A HREF=oview/binprot.htm> Vixen binary protocol</A>
 * can be saved and loaded from files.
 *
 * Once a scene has been loaded from a file, it maintains a dictionary which
 * allows you to rapidly access objects based on their name. This dictionary
 * remains available until the stream is closed. The content converters
 * use the convention of prepending the name of the scene manager output file to the
 * object name specified in the modeling tool. Exporting a model called
 * \b test_box to a file called \b world.vix would name the object
 * \b world.test_box (Not all Vixen objects are reliably assigned names
 * so you cannot retrieve every stream object this way.)
 *
 * @ingroup vixen
 * @see Messenger FileStream World::LoadAsync FileLoader
 */
class FileMessenger : public Messenger
{
public:
	FileMessenger(const TCHAR* filename = NULL);

	VX_DECLARE_CLASS(FileMessenger);
	bool			Open(const TCHAR* name, int mode = Core::Stream::OPEN_RW);
	bool			Close();
	int				CanSave(const SharedObj* obj, int opts);
	int				Attach(const SharedObj*, int flags = 0);
	SharedObj*		Create(uint32 classid, int handle = 0);
	Messenger&	OutObj(const SharedObj* obj);
};

} // end Vixen