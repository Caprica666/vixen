#include "vix_all.h"

#include <maya/MFnAttribute.h>
#include <maya/MItDependencyGraph.h>

/*!
 * @fn meConvObj::meConvObj(const MDagPath& dagPath)
 * @param dagPath	Maya DAG path of node to convert
 *
 * Creates a converter to convert the given Maya node to Vixen data structures.
 *
 * @see meConvObj::MakeConverter meConvObj::Make meConvObj::Convert
 */
meConvObj::meConvObj(const MDagPath& dagPath) :  Group()
{
    MFnDagNode	node(dagPath, &m_Status);

	DoConvert = false;
    DoLink = false;
    m_VXObj = (SharedObj*) NULL;
	m_Instance = 0;
    if (m_Status != MS::kSuccess)
		ME_ERROR_RETURN(("ERROR: MFnDagPath constructor failed for %s", m_DagPath.fullPathName()));
	SetDagPath(dagPath);
 	m_MayaObj = node.object();
    DoLink = true;
	DoScan = 0;
	Core::String name(node.name().asChar(), ME_MAX_NAME);
	m_MayaName = (const char*) name;
}

meConvObj::meConvObj(MObject& obj)
  : Group(), m_MayaObj(obj)
{
    MFnDependencyNode node(m_MayaObj, &m_Status);

    DoConvert = false;
    DoLink = false;
    m_Instance = 0;
    m_VXObj = (SharedObj*) NULL;
    if (m_Status != MS::kSuccess)
		ME_ERROR_RETURN(("ERROR: MFnDependencyNode constructor failed for object of type %s", m_MayaObj.apiTypeStr()));
    DoLink = true;
	if (m_MayaObj.hasFn(MFn::kDagNode))
	{
		MFnDagNode dagnode(m_MayaObj, &m_Status);
		SetDagPath(dagnode.dagPath());
	}
	Core::String name(node.name().asChar(), ME_MAX_NAME);
	m_MayaName = (const char*) name;

}

meConvObj::~meConvObj()
{
	m_VXObj = (SharedObj*) NULL;
}

/*!
 * @fn void meConvObj::SetDagPath(const MDagPath& dagpath)
 * @param dagpath	Maya DAG path to use for this converter
 *
 * Changes the DAG path of this converter. This call should be used
 * with care and only when the new DAG path references the same Maya
 * object as the previous one. The Vixen node associated with
 * this converter is NOT changed.
 */
void meConvObj::SetDagPath(const MDagPath& dagpath)
{
    m_DagPath = dagpath;
    if (m_DagPath.isInstanced())		// if instanced, tell it which instance
        m_Instance = m_DagPath.instanceNumber();
    else
		m_Instance = 0;
}

void meConvObj::SetMayaName(const char* name)
{
    m_MayaName = name;
}

/*!
 * @fn SharedObj* meConvObj::Make()
 *
 * Creates one or more Vixen objects from the Maya node associated with this converter.
 * The base implementation always returns NULL.
 *
 * @return root Vixen object created, NULL if Maya node cannot be converted
 *
 * @see meConvObj::Convert
 */
SharedObj* meConvObj::Make()
{
    return NULL;
}

/*!
 * @fn int meConvObj::Convert(meConvObj* = NULL)
 *
 * Converts the Maya node into Vixen data structures and attaches the Vixen
 * object(s) to this converter. It is the responsibility of the caller to attach the new
 * Vixen node(s) into the overall hierarchy.
 * This base implementation does nothing.
 *
 * @return -1 on error, 0 if Maya node is skipped, 1 if successful conversion
 *
 * @see meConvObj::Make meConvObj::MakeConverter
 */
int meConvObj::Convert(meConvObj*)
{
    return -1;
}

/*!
 * @fn int meConvObj::Link(meConvObj* convparent)
 *
 * Links the Vixen data structures together if necessary.
 * This base implementation does nothing.
 *
 * @return -1 on error, 0 if no link, 1 if successful linkage
 */
int meConvObj::Link(meConvObj* convparent)
{
    return 0;
}

/*!
 * @fn void meConvObj::SetName(SharedObj* psobj, const char* mayaname = NULL)
 * @param psobj		Vixen object to name, if NULL the object associated
 *					with this converter is used
 * @param mayaname	Maya name of object, if NULL the Maya name is derived from
 *					the DAG path or Maya object
 *
 * Sets the name of the Vixen object based on the Maya node name.
 * The Vixen node name is the Maya name prefixed by the basename of the output file.
 * For example, if we are exporting to /usr/aw/maya/scenes/myscene.vix and the Maya
 * name of the node is Cube1, the Vixen name will be myscene.Cube1
 *
 */
void meConvObj::SetName(SharedObj* vxobj, const char* mayaname)
{
    if (vxobj == NULL)
		vxobj = m_VXObj;
    if (vxobj == NULL)
        return;
    if (!mayaname)
		mayaname = GetMayaName();
    if (!mayaname)
		return;
    Core::String namebuf(Exporter->FileBase);
    namebuf += TEXT(".");
	namebuf += mayaname;
    vxobj->SetName(namebuf);
}

/*!
 * @fn meConvNode* meConvObj::MakeConverter(MObject& mayaObj)
 * @param mayaObj	Maya object to make converter for
 *
 * Makes a converter for the given Maya object. This is a class factory for converters.
 * Each Maya node type uses a different meConvObj subclass for converting it to Vixen.
 *
 * @see meConvNode::MakeConverter meConvEng::MakeConverter
 */
meConvObj* meConvObj::MakeConverter(MObject& mayaObj)
{
    MStatus				status = MS::kSuccess;
    MFnDependencyNode	node(mayaObj, &status);
 	meConvObj*			converter = NULL;

	converter = meConvEng::MakeConverter(mayaObj);
	if (converter)
		return converter;
	return meConvShader::MakeConverter(mayaObj);
}

void meConvObj::PrintPlugs(MObject& obj)
{
    MStatus status;
    MFnDependencyNode node(obj, &status);

    if (!status)
		return;
    for (uint32 a = 0; a < node.attributeCount(); ++a)
    {
		MObject attr(node.attribute(a));
		MPlug p = node.findPlug(attr, &status);
		MString val;
		MPlugArray connections;

		if (status != MS::kSuccess)
			continue;
		if (p.connectedTo(connections, true, true, &status) && (status == MS::kSuccess))
			for (uint32 i = 0; i < connections.length(); ++i)
				meLog(3, "\t%s %s %s", p.name().asChar(), p.isDestination() ? "->" : "<-", connections[i].name().asChar());
   }
}

meConnIter::meConnIter(MObject& mayaobj)
{
    Reset(mayaobj);
}

meConnIter::meConnIter(MObject& mayaobj, const char* attrname, int type)
{
	Reset(mayaobj, attrname, type);
}

meConnIter::meConnIter(const MPlug& plug, int type)
{
	Reset(plug, type);
}

void meConnIter::Reset(MObject& mayaobj)
{
	MStatus	status;
	MPlug	plug;
	MFnDependencyNode node(mayaobj, &status);

	m_Index = -1;
	Debug = false;
	if ((status == MS::kSuccess) && node.getConnections(m_Connections))
		m_Index = 0;
}

void meConnIter::Reset(MObject& mayaobj, const char* attrname, int type)
{
    MStatus	      status;
    MFnDependencyNode node(mayaobj, &status);
    MPlug	      plug;

    m_Index = -1;
	Debug = false;
    if (!status || !attrname)
		return;
    plug = node.findPlug(attrname);
	Reset(plug, type);
}

void meConnIter::Reset(const MPlug& plug, int type)
{
    m_Index = -1;
	Debug = false;
	switch (type)
	{
		case SOURCE:
		if (plug.connectedTo(m_Connections, false, true))
			m_Index = 0;
		break;

		case DEST:
		if (plug.connectedTo(m_Connections, true, false))
			m_Index = 0;
		break;

		default:
		if (plug.connectedTo(m_Connections, true, true))
			m_Index = 0;
	}
}

bool meConnIter::Next(MPlug& plug)
{
    if (m_Index < 0)
		return false;
    if (m_Index >= (int) m_Connections.length())
    {
		m_Index = -1;
		return false;
    }
    m_CurPlug = plug = m_Connections[m_Index++];	
    m_PlugName = plug.name().asChar();
	MObject& obj = plug.node();
	ME_TRACE(Debug, ("Plug: %s %s", m_PlugName, obj.apiTypeStr()) );
    return true;
}


bool meConnIter::Next(MObject& obj, MFn::Type mayatype)
{
	MPlug plug;

	while (Next(plug))
	{
		obj = plug.node();
		if (obj.hasFn(mayatype))
			return true;
	}
	return false;
}

bool meConnIter::Next(MPlug& plug, const TCHAR* attrname)
{
	while (Next(plug))
	{
		Core::String name(plug.name().asChar());

		if (name.Find(attrname) >= 0)
			return true;
	}
	return false;
}

bool meConnIter::FindSourceConnection(const MPlug& srcplug, MPlug& dstplug, MFn::Type mayatype)
{
	MPlugArray connected;

	if (!srcplug.connectedTo(connected, false, true))
		return false;
	for (int i = 0; i < connected.length(); ++i)
	{
		dstplug = connected[i];
		if (dstplug.node().hasFn(mayatype))
			return true;
	}
     return false;
}

bool meConnIter::FindDestConnection(const MPlug& srcplug, MPlug& dstplug, MFn::Type mayatype)
{
	MPlugArray connected;

	if (!srcplug.connectedTo(connected, true, false))
		return false;
	for (int i = 0; i < connected.length(); ++i)
	{
		dstplug = connected[i];
		if (dstplug.node().hasFn(mayatype))
			return true;
	}
     return false;
}

bool meConnIter::Next(MPlug& dstplug, MFn::Type mayatype)
{
	MPlug srcplug;
    while (Next(srcplug))
    {
		if (meConnIter::FindDestConnection(srcplug, dstplug, mayatype))
			return true;
    }
    return false;
}

bool meConnIter::NextSource(MPlug& dstplug, MFn::Type mayatype)
{
	MPlug srcplug;
    while (Next(srcplug))
    {
		if (meConnIter::FindSourceConnection(srcplug, dstplug, mayatype))
			return true;
    }
    return false;
}

bool meConnIter::FindChild(MPlug& parent, const char* name, MPlug& child)
{
    MObject attrobj;
    MString mayaname = name;

    for (uint32 j = 0; j < parent.numChildren(); ++j)
    {
		child = parent.child(j);
		attrobj = child.attribute();
		MFnAttribute attr(attrobj);

		if (mayaname == attr.name().asChar())
			return true;
    }
    return false;
}
