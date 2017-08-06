#pragma once

namespace Vixen {

/*!
 * @class ExtModel
 * @brief References a model which comes from an external file.
 *
 * External models can be used in the scene just like regular models.
 * When an external model is added to the scene, it has no children.
 * When it is visited for the first time,
 * a request is made to the running application to load the file
 * associated with the external model. After the file load has
 * completed, the objects in the file become children of the external model.
 * They will then appear in the scene.
 *
 * You can change the name of the file to load at run time, effectively
 * replacing it in the scene without perturbing the hierarchy. Objects
 * can be unloaded when no longer in view as a crude form of culling.
 * Because switching files or unloading replaces all of its children,
 * external models are only useful as leaf nodes.
 *
 */
#define	XMOD_IsLoaded	32	// true if file has been loaded
#define	XMOD_IsLoading	64	// true while file is loading

class ExtModel: public Model
{
public:
	VX_DECLARE_CLASS(ExtModel);
	ExtModel();

	const TCHAR* GetExtName() const;	//!< Sets the name of the object to load from the external file.

	const TCHAR* GetFileName() const;	//!< Returns name of file with content.
	bool		IsLoaded() const;		//!< Returns  true if external model has been loaded.
	void		Unload();				//!< Unload in-memory representation.
	int			GetOptions() const;		//!< Returns external model behavior options.

	//! Set external name (object name from content file).
	void		SetExtName(const TCHAR *name);

	//! Establish name of file containing external objects.
	void		SetFileName(const TCHAR *name);

	//! Establish external model behavior options.
	void		SetOptions(int);

	//! Load external model from file.
	void		Load(const TCHAR* filename = NULL);

	//!< Enable automatic loading of external models.
	static void	AutoLoadAll(bool);

	// overrides
	virtual	int			Save(Messenger&, int) const;
	virtual	bool		Do(Messenger& s, int op);
	virtual	bool		Copy(const SharedObj* src);
	virtual	intptr		Cull(const Matrix* m, Scene* scene);
	virtual	bool		OnEvent(Event* event);

	enum
	{
		AUTOLOAD = 1,	//!< automatically load the model
		REMOVE = 2,		//!< add the loaded copy to the scene (default is to clone the loaded copy)
	};

	enum Opcode
	{
		XMOD_SetFileName = Model::MOD_NextOp,
		XMOD_SetOptions,
		XMOD_SetExtName,
		XMOD_Load,
		XMOD_Unload,
		XMOD_NextOp = Model::MOD_NextOp + 10,	
	};

protected:
	void		MakeObjName(TCHAR* namebuf);

	static bool		s_AutoLoadAll;
	int32			m_Options;
	Core::String	m_FileName;
	Core::String	m_ExtName;
};

inline ExtModel::ExtModel() : Model()
{
	m_Options = s_AutoLoadAll ? AUTOLOAD : 0;
}

inline const TCHAR* ExtModel::GetFileName() const
	{ return m_FileName; }

inline const TCHAR* ExtModel::GetExtName() const
	{ return m_ExtName; }

inline int ExtModel::GetOptions() const
	{ return m_Options; }

inline bool ExtModel::IsLoaded() const
{
	return (m_Options & XMOD_IsLoaded) != 0;
}

} // end Vixen