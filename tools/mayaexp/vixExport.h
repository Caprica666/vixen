#pragma once

/*
 * Vixen includes
 */

#include "metypes.h"
#include "mepreview.h"
class meConvObj;

/*
 * Maya includes
 */
#include <maya/MPxCommand.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MPoint.h>
#include <maya/MDagPathArray.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatArray.h>
#include <maya/MPlug.h>
#include <maya/MPxFileTranslator.h>
#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MDistance.h>
#include <maya/MIntArray.h>
#include <maya/MFnMesh.h>
#include <maya/MMatrix.h>
#include <maya/MTransformationMatrix.h>

typedef Ref<meConvObj>	ConvRef;

inline bool Core::Dict< MObject, ConvRef, BaseDict >::CompareKeys(const MObject& knew, const MObject& kdict)
{
    return knew == kdict;
}

/*
 * This routine is very tricky because it accesses private data from the Maya object
 * to make a hash key. The "ptr" field is the pointer to the internal Maya data structure
 * and is the same if the MObject references the same data.
 * class MObject {
 * private:
 *    MPtrBase*  ptr;		// unique key for object
 *    int        tp;
 * };
 */
inline uint32 Core::Dict< MObject, ConvRef, BaseDict >::HashKey(const MObject& obj) const
{
    const intptr* ptr = (const intptr*) &obj;
    intptr key = *ptr;
    return HashInt((uint32) (key >> 2));
}

inline bool Core::Dict< MDagPath, ConvRef, BaseDict >::CompareKeys(const MDagPath& knew, const MDagPath& kdict)
{
	MString	newpath = knew.fullPathName();
	MString dictpath = kdict.fullPathName();
	const char* path1 = newpath.asChar();
	const char* path2 = dictpath.asChar();
    return strcmp(path1, path2) == 0;
}

inline uint32 Core::Dict< MDagPath, ConvRef, BaseDict >::HashKey(const MDagPath& path) const
{
	MString str = path.fullPathName();
    const char* key = str.asChar();
    return HashStr(key);
}

typedef Dictionary< MObject, ConvRef > meObjDict;
typedef Dictionary< MDagPath, ConvRef >  meDagDict;

class meConvEng;

/*!
 * @class vixExporter
 * @brief Maya Exporter to .VIX format
 *
 * Provides the Maya exporter plugin interface for the Vixen exporter.
 * This class walks the Maya scene graph and converts the Maya nodes into
 * Vixen in-memory data structures. This Vixen scene graph is then
 * saved in a .VIX file so it can be viewed with the Vixen viewer.
 *
 * The "DoPreview" option on the exporter will create a new Maya window
 * and display the Vixen scene within that window. Vixen display
 * is handled by the vixViewer plug-in.
 *
 * @see meConvObj meDagDict vixViewer mePreview
 */
class vixExporter : public MPxFileTranslator
{
public:
	vixExporter();
	virtual         ~vixExporter();
	static void*    creator();

	MStatus		reader(const MFileObject& file, const MString& optionsString, FileAccessMode mode);
	MStatus		writer(const MFileObject& file, const MString& optionsString, FileAccessMode mode );
	bool		haveReadMethod () const;
	bool		haveWriteMethod () const;
	MString		defaultExtension () const;
	MFileKind	identifyFile(const MFileObject& fileName, const char* buffer, short size) const;
	void		initVixen();
	meConvObj*	makeNode(MDagPath& dagPath);
	meConvObj*	makeOne(MObject& mayaObj);
	meConvObj*	linkNode(MDagPath& dagPath, meConvObj* convparent = NULL);
	meConvObj*	linkOne(MObject& mayaObj, meConvObj* convparent = NULL);
	bool		scanInputs(MObject& mayaObj);

private:
	void		parseOptions(const MString& options);
	MStatus		exportSelected();
	MStatus		exportAll();
	MStatus		convertAll();
	MStatus		animateAll();
	void		setCamera();
	bool		writeVixen(const char* fname);
	void		pruneScene(Scene* scene);

public:
	bool		DupVerts;		//! true to enable duplication of vertices (default)
	bool		DoPreview;		//! true to enable visual preview
	bool		DoTextures;		//! true to export textures and texture coords
	bool		DoAnimation;	//! true to export animation engines
	bool		DoGeometry;		//! true to export geometry
	bool		DoSkinning;		//! true to export skinned animation, else rigid body only
	int			TexWidth;		//! pixel width of exported RGB textures
	int			TexHeight;		//! pixel height of exported RGB textures

	int			LogLevel;		//! level of information logging
	int			LogDebug;		//! output logfile to debugger / stderr
	DebugOut*	LogFile;		//! file descriptor of output log
	char		FilePath[100];	//! directory of output file
	char		FileBase[100];	//! base name of output file (no path or ext)

	meDagDict	MayaNodes;		//! dictionary of Maya DAG node converters
	meObjDict	MayaObjects;	//! dictionary of Maya Dependency node converters (shaders, engines)
	Ref<Scene>	Scene;			//! Vixen scene
	Ref<Model>	Root;			//! root of Vixen model hierarchy
	Ref<Engine>	SimRoot;		//! root of Vixen engine
	MSelectionList	Selected;	//! selected nodes in Maya

private:
	bool		b_LinkPass;		// true if link pass (vs make pass)
	bool		b_SelectedOnly;
};

extern int Debug;

