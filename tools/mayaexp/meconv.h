#pragma once

#include <maya/MPlug.h>
#include <maya/MPlugArray.h>

/*!
 * @class meConvObj
 * @brief Base class to convert between Maya and Vixen data structures.
 *
 * The base implementation does nothing. Subclasses deal with the different
 * types of Maya objects. For example, meConvNode handles hierarchy linkage and
 * conversion of the transformation matrix. meConvPoly deals with different types of geometry
 * and rendering state. Converters are reference counted so they can be
 * shared in the global Maya name dictionary.
 *
 * @see meDAGDict vixExporter meConvNode
 */
class meConvObj  : public Group
{
public:
    meConvObj(const MDagPath& mpath);
    meConvObj(MObject& obj); 

    //! Get the Maya DAG path for this converter.
    const MDagPath&	GetDagPath() const	{ return m_DagPath; }

    //! Set the Maya DAG path for this converter.
    void		SetDagPath(const MDagPath& path);

    //! Get the Vixen object created by this converter.
    SharedObj*	GetVXObj()				{ return m_VXObj; }

    //! Get the Maya object this converter will convert.
    MObject&	GetMayaObj()			{ return m_MayaObj; }

    //! Destructor - deletes Vixen objects created by the converter.
    virtual		~meConvObj();

    //! Make a Vixen object of the appropriate type.
    virtual SharedObj*	Make();

    //! Convert the Maya object into one or more Vixen nodes.
    virtual int		Convert(meConvObj* = NULL);

    //! Link Vixen nodes together if necessary.
    virtual int		Link(meConvObj* convparent = NULL);

    //! Get Maya name of object to be converted.
    const char*		GetMayaName();

    //! Set Maya name of node
    void			SetMayaName(const char* name);

    virtual void	SetName(SharedObj* obj = NULL, const char* mayaname = NULL);

	//! Class factory for Maya -> Vixen converters
	static meConvObj*	MakeConverter(MObject& mayaObj);

	//! Print Maya plug connections
    static void		PrintPlugs(MObject& obj);

    bool			DoConvert;		//!< true if Convert() should be called
    bool			DoLink;			//!< true if Vixen object linked to others
	int				DoScan;			//!< true to scan inputs for this object

protected:
    MDagPath		m_DagPath;
    MStatus			m_Status;
    ObjRef			m_VXObj;
    MObject			m_MayaObj;
    MString			m_MayaName;
    int				m_Instance;
};

inline const char* meConvObj::GetMayaName()
{
    if (m_MayaName.length() == 0)
        return NULL;
    return m_MayaName.asChar();
}


class meConnIter
{
	enum
	{
		DEST = 1,
		SOURCE = 2,
		BOTH = 3
	};

public:
    meConnIter(MObject& mayaobj, const char* attrname, int type = DEST);
	meConnIter(MObject& mayaobj);
    meConnIter(const MPlug& plug, int type = DEST);

    bool Next(MPlug& plug);
	bool Next(MPlug& plug, MFn::Type mayatype);
	bool Next(MObject& obj, MFn::Type mayatype = MFn::kDependencyNode);
	bool Next(MPlug& obj, const TCHAR* attrname);
	bool NextSource(MPlug& plug, MFn::Type mayatype);
	const MPlug&	CurPlug()	{ return m_CurPlug; }
    void Reset(MObject& mayaobj);
    void Reset(MObject& mayaobj, const char* attrname, int type = DEST);
    void Reset(const MPlug& plug, int type = DEST);
    const char*	PlugName()	{ return m_PlugName; }
    static bool FindChild(MPlug& parent, const char* name, MPlug& child);
	static bool FindSourceConnection(const MPlug& srcplug, MPlug& dstplug, MFn::Type mayatype);
	static bool FindDestConnection(const MPlug& srcplug, MPlug& dstplug, MFn::Type mayatype);
	bool		Debug;

protected:
    MPlugArray		m_Connections;
    int				m_Index;
	int				m_ConnType;
    Core::String	m_PlugName;
	MPlug			m_CurPlug;
};

