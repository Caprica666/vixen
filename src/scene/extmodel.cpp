#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(ExtModel, Model, VX_ExtModel);

bool ExtModel::s_AutoLoadAll = true;

void ExtModel::AutoLoadAll(bool v)
{
	s_AutoLoadAll = v;
}

/*!
 * @fn void ExtModel::SetFileName(const TCHAR* filename)
 * @param filename name of 3D content file containing external model
 *
 * Sets the name of the file which contains the externally
 * referenced object. This should be a .Vixen scene file that
 * contains an object with the name specified by  SetExtName.
 * If the  XMOD_AutoLoad option is enabled, this file will
 * automatically load when the model is visited in the scene graph.
 *
 * The external name remains in effect even if the filename is changed.
 * This is to permit the same name to be used from different
 * files as you might do if you wanted to switch from lores to hires objects.
 *
 * @see ExtModel::Load ExtModel::SetXrefName
 */
void ExtModel::SetFileName(const TCHAR* filename)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_ExtModel, XMOD_SetFileName) << this << filename;
	VX_STREAM_END(  )

	if (filename && (m_FileName.CompareNoCase(filename) == 0))
		return;
	m_FileName = filename;
}

/*!
 * @fn void ExtModel::SetExtName(const TCHAR* name)
 * @param name	string name of object to load, cannot be NULL
 *
 * A single external file may contain multiple objects used as
 * separate external models in the same scene. The input name is assumed
 * to \b not have a filename prefix.
 *
 * If no object name is specified for this external reference,
 * the object loaded is assumed to be the same as the name of
 * the external reference object in the current scene (as
 * established by SharedObj::SetName.
 *
 * @see ExtModel::SetFileName SharedObj::SetName ExtModel::Load
 */
void ExtModel::SetExtName(const TCHAR* name)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_ExtModel, XMOD_SetExtName) << this << name;
	VX_STREAM_END(  )
	m_ExtName = name;
}


/*!
 * @fn void ExtModel::SetOptions(int opts)
 * @param opts	one or more option flags
 *
 * The option flags controlling load and display of external references.
 * @code
 *	ExtModel::AUTOLOAD	automatically load external references
 *						when they are visited in the scene graph
 *	ExtModel::REMOVE	removes the object from the xref hierarchy
 *						instead of cloning it. only use this option
 *						if the xref file is not referenced multiple
 *						times in the scene. default is to clone.
 *	XMOD_IsLoaded		turned on after a file has been loaded
 *						or to suppress loading
 * @endcode
 *
 * @see ExtModel::Load ExtModel::SetExtName
 */
void ExtModel::SetOptions(int opts)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_ExtModel, XMOD_SetOptions) << this << int32(opts);
	VX_STREAM_END( )

	m_Options = opts;
}

/*!
 * @fn void ExtModel::Load(const TCHAR* filename)
 * @param filename	name of file to load. If NULL, the file established
 *					by SetFileName is used. This becomes the current filename.
 *
 * The file containing the external object is loaded if the object cannot already
 * be found in the scene. External references which have been previously
 * loaded are kept around attached to the scene even if they are not
 * displayed so they may be resued again later.
 *
 * Scene manager objects which come from files usually follow a naming
 * convention which prepends the name of the file to and
 * name of the object in the modeling package. For example,
 * an external reference in a 3DS Max file called "Box01"
 * exported to file "foo.vix" will be called "foo.Box01" in the scene manager.
 * Given the name of the input file and the name of the external
 * reference object in the current scene, the scene manager determines the
 * name the object would have if it were already loaded and
 * available in the current scene. If this is the case, that
 * object is used instead of loading the file again. You can
 * override this default behavior and set the exact name of the
 * externally referenced object to use from the file with  SetExtName. 
 * 
 * If the file must be loaded, a request is made to the application
 * to read in the data. When the file has finished loading,
 * this external model will get a scene load event (EVENT_Load).
 *	
 * Then the object in the loaded file with the same name as this external
 * reference is added as a child of this model. It replaces any
 * previous children (which are dereferenced). You should not add children
 * to external models you plan to reload. They will disappear
 * when the file is reloaded.
 *
 * @see ExtModel::SetFileName ExtModel::Unload ExtModel::SetExtName
 */
void ExtModel::Load(const TCHAR* filename)
{
	VX_STREAM_BEGIN(str)
		*str << OP(VX_ExtModel, XMOD_Load) << this << filename;
	VX_STREAM_END( )

	if (m_Options & XMOD_IsLoading)
		return;
	if (filename == NULL)				// if no input filename, use current file
	{
		filename = m_FileName;
		if (filename == NULL)
			return;
	}
	else
		SetFileName(filename);			// replace current filename
/*
 * Get full object name and see if it has already been loaded.
 * The filename prefix is the base of the external file and
 * the objectname is the external name (SetExtName).
 */
	Messenger*	disp = GetMessenger();
	TCHAR		objname[VX_MaxPath];
	Group*		mod;

	MakeObjName(objname);
	if (mod = (Group*) disp->Find(objname))
	{
		Unload();						// unload old contents
		if (m_Options & REMOVE)		// clone or remove?
			mod->Remove(Group::UNLINK_NOFREE);
		else
			mod = (Group*) mod->Clone();
		Append(mod);
		m_Options |= XMOD_IsLoaded;
		return;
	}
	disp->Observe(this, Event::LOAD_SCENE, NULL); 
/*
 * content not loaded yet - have the application load the file and set
 * up to handle the load event (in OnEvent)
 */
	m_Options |= XMOD_IsLoading;
	m_Options &= ~XMOD_IsLoaded;
	World3D::Get()->LoadAsync(filename, this);
}

/*!
 * @fn void ExtModel::Unload()
 *
 * Unloads the current visual representation of the external reference
 * by deleting all of its children (even those which did not come
 * from the external file). This function does  not completely
 * delete the object - it is still attached to the file loader.
 * To eliminate the object completely, you need to unload the
 * external file as well.
 *
 * @see ExtModel::SetFileName ExtModel::Load World::Unload
 */
void ExtModel::Unload()
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_ExtModel, XMOD_Unload) << this ;
	VX_STREAM_END(  )

	Model* child;

	while (child = First())
		child->Remove(UNLINK_FREE);
	m_Options &= ~XMOD_IsLoaded;
}


/****
 *
 * Handles the LOAD_SCENE (scene is loaded) event which signals
 * the external Vixen file has been loaded
 *
 ****/
bool ExtModel::OnEvent(Event* ev)
{
	TCHAR			objname[VX_MaxPath];
	Model*		xroot;
	Scene*		scene;
	LoadSceneEvent*	se = (LoadSceneEvent*) ev;

	if (ev->Code != Event::LOAD_SCENE)
		return false;
	if (!(m_Options & XMOD_IsLoading))
		return false;
	if (m_Options & XMOD_IsLoaded)
		return false;
	scene = (Scene*) (SharedObj*) se->Object;
	if (!scene || !scene->IsClass(VX_Scene))
		return true;
	if (scene->GetName() == NULL)
		return true;
	if (!se->Sender->IsClass(VX_ExtModel))
		return false;
	const TCHAR* p = se->FileName;
	size_t n = STRLEN(p) - STRLEN(m_FileName);	// compare file name lengths
	if (n < 0)								// xref file must match event file at the end 
		return false;
	if (m_FileName.CompareNoCase(p + n) != 0)
		return false;
	xroot = scene->GetModels();
	if (xroot == NULL)
		return true;
	VX_PRINTF(("ExtModel::OnEvent LOAD_SCENE %s\n", m_FileName));
/*
 * Make up the name to search for. Prefix is the base of the file being
 * loaded (the Xref's filename) and suffix is the object name of the Xref
 * Look for the object in the file loaded. If not found, it is not
 * this Xref's load event
 */
	MakeObjName(objname);
	xroot = (Model*) xroot->Find(objname, Group::FIND_DESCEND | Group::FIND_EXACT);
	if (xroot == NULL)
		return true;
/*
 * Remove previous children and append new object
 */
	Unload();							// unload previous children
	if (m_Options & REMOVE)				// clone or remove?
		xroot->Remove(Group::UNLINK_NOFREE);
	else
		xroot = (ExtModel*) xroot->Clone();
	xroot->Reset();						// remove matrix from file scene
	xroot->SetFlags(IsShared());		// inherit parent's sharing options
	SetBound((Sphere*) NULL);			// let bounds auto adjust
	GetMessenger()->Define(objname, xroot);
	Append(xroot);						// append to parent
	m_Options |= XMOD_IsLoaded;			// mark as loaded
	m_Options &= ~XMOD_IsLoading;
//	m_Hints |= xroot->GetHints();		// inherit hints
	return false;	
}

intptr ExtModel::Cull(const Matrix* m, Scene* scene)
{
	if ((m_Options & ~REMOVE) == AUTOLOAD)
	{
		if (!m_FileName.IsEmpty())
			((ExtModel*) this)->Load(m_FileName);
		return DISPLAY_NONE;
	}
	if (m_Options & XMOD_IsLoading)
		return DISPLAY_NONE;
	return Model::Cull(m, scene);
}

int ExtModel::Save(Messenger& s, int o) const
{
//	if (!World3D::Get()->DoAsyncLoad)	// disable autoload if distributed
//	   {
//		ExtModel* cheat = (ExtModel*) this;
//		if (!IsParent())			// force load if saving distributed xref
//			cheat->Load();
//	   }
	int32	h = Model::Save(s, o);
	int32	opts = GetOptions();
	if (h <= 0)
		return h;
	if (IsGlobal())					// disable autoload if distributed
		opts &= ~AUTOLOAD;
	s << OP(VX_ExtModel, XMOD_SetOptions) << h << opts;
	s << OP(VX_ExtModel, XMOD_SetFileName) << h << GetFileName();
	s << OP(VX_ExtModel, XMOD_SetExtName) << h << GetExtName();
	return h;
}

bool ExtModel::Do(Messenger& s, int op)
{
	size_t		n;
	int32		opt;
	TCHAR		name[VX_MaxPath];
	TCHAR		path[VX_MaxPath];

	switch (op)
	{
		case XMOD_SetFileName:
		s >> name;
		n = STRLEN(name);
//		name[n-3] = 'p';
//		name[n-2] = 's';
//		name[n-1] = 'm';
		s.GetPath(name, path, VX_MaxPath);
		SetFileName(path);
		break;

		case XMOD_SetExtName:
		s >> name;
		SetExtName(name);
		break;		

		case XMOD_Load:
		s >> name;
		s.GetPath(name, path, VX_MaxPath);
		Load(path);
		break;

		case XMOD_Unload:
		Unload();
		break;

		case XMOD_SetOptions:
		s >> opt;
		SetOptions(opt);
		break;

		default:
		return Model::Do(s, op);
	}
	return true;
}

bool ExtModel::Copy(const SharedObj* src_obj)
{
	const ExtModel* src = (ExtModel*) src_obj;
	if (!Model::Copy(src_obj))
		return false;
	if (!src->IsClass(VX_ExtModel))
		return true;
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	m_FileName = src->m_FileName;
	m_ExtName = src->m_ExtName;
	m_Options = src->m_Options & (XMOD_IsLoading | XMOD_IsLoaded);
	return true;
}

/****
 *
 * MakeObjName(TCHAR* buffer)
 *
 * Get full object name and see if it has already been loaded.
 * filename prefix is the base of the XREF file and objectname
 * is created from the objectname of the XREF object name and
 * the XREF file base.
 *	XREF file = rubbertree.vix
 *	XREF name = plant
 *	objname to look for = rubbertree.plant
 *
 ****/
void ExtModel::MakeObjName(TCHAR* objname)
{
	const TCHAR* s = m_FileName;
	const TCHAR* filebase;
	const TCHAR* t = s;
	TCHAR*		e = STRRCHR((TCHAR*) s, TEXT('.'));
	size_t		n = STRLEN(s);

	do							// find the last slash or backslash
	{
		s = t;
		size_t p = STRCSPN(s, TEXT("/\\"));
		t += p;
	}
	while (*t++);
	if (e)						// find the extension
		n = e - s;
	else
		n -= s - m_FileName;
	filebase = s;				// -> basename of file
	for (size_t i = 0; i < n; ++i)
		objname[i] = tolower(s[i]);
	objname[n] = 0;

	s = GetExtName();			// user specified object name to look for?
	if ((s == NULL) || (*s == 0))
	{							// no, assume it is the same as this node
		s = GetName();			// except for the file prefix
		if (s)
		{
			e = STRCHR((TCHAR*) s, TEXT('.'));	// skip the file prefix
			if (e)
				s = e + 1;
		}
		else
			s = TEXT("root");
	}
	STRCAT(objname, TEXT("."));
	STRCAT(objname, s);		// append object name to file base
}

}	// end Vixen