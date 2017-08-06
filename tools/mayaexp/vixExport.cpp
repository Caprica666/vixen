#include "vix_all.h"
#include <iostream>
#include <fstream>
#include "mepreview.h"

EXPORT mePreview* VIXPreview = NULL;

#include <maya/MFnPlugin.h>
#include <maya/M3dView.h>
#include <maya/MItDependencyGraph.h>
#include<maya/MAnimControl.h>
#include<maya/MTime.h>

const char *const vixOptionScript = "vixExportOptions";
const char *const vixDefaultOptions =
    "preview=0;"
    "width=256;"
    "height=256;"
    "textures=1;"
    "loglevel=3;"
	"animation=1;"
	"duplicateverts=1;"
    ;

vixExporter* Exporter = NULL;

#ifdef ME_VIXENVIEWER

class VXExportWorld : public World3D
{
public:
	VXExportWorld(): World3D(), Options(0)	{ DoAsyncLoad = false; }

	void AddEngine(Engine* eng)
	{
		Engine* simroot = Exporter->Scene->GetEngines();
		if (simroot)
			simroot->Append(eng);
	}
protected:
	int		Options;
};
#endif


/*!
 * @fn vixExporter::vixExporter()
 * Constructor for Vixen exporter.
 * Initializes state of converter, including state of converter parameters accessible via UI.
 *
 * Performs one-time initialization of Vixen.
 */
vixExporter::vixExporter()
{
#ifdef _SAFE
    LogLevel = 3;
    LogDebug = 3;
#else
    LogLevel = 3;
    LogDebug = 2;
#endif

#ifdef ME_VIXENVIEWER
	DoPreview = 1;
    VIXPreview = new mePreview();
#else
	DoPreview = 0;
#endif
    LogFile = NULL;
	DoAnimation = false;
	DoSkinning = true;
	DoGeometry = true;
    DoTextures = false;
    TexWidth = 512;
    TexHeight = 512;
    b_LinkPass = false;
	b_SelectedOnly = false;
	REGISTER_CLASS(KeyFramer);
	REGISTER_CLASS(Transformer);
	REGISTER_CLASS(Skeleton);
	REGISTER_CLASS(Skin);
	REGISTER_CLASS(Deformer);
	REGISTER_CLASS(Morph);
	REGISTER_CLASS(MeshAnimator);
    initVixen();
}

/*!
 * @fn vixExporter::~vixExporter()
 * Destructor for Maya to Vixen exporter.
 * Deletes the visual preview shared data structure and attempt to release
 * Vixen resources without exiting.
 *
 * @see vixExporter::initVixen
 */
vixExporter::~vixExporter()
{
	World3D* world = World3D::Get();
	if (world)
	{
		world->Stop();
		world->Delete();
	}
    delete VIXPreview;
    VIXPreview = NULL;
}

/*!
 * @fn void* vixExporter::creator()
 *
 * Static function called by the Maya infrastructure to make an instance
 * of the Maya -> Vixen export plugin.
 *
 * Vixen initialization is performed here an in vixExporter::initVixen
 * (which is called from the constructor). Upon return from this routine,
 * all one-time only initialization of Vixen has been done.
 */
void* vixExporter::creator()
{
    assert(Exporter == NULL);
    return Exporter = new vixExporter();
}

/*!
 * @fn MStatus vixExporter::reader(const MFileObject&, const MString&, FileAccessMode)
 *
 * Called when the plugin is used as a reader. This plugin does not support
 * reading of .VIX files - only writing of them.
 */
MStatus vixExporter::reader(const MFileObject&, const MString&, FileAccessMode)
{
    ME_ERROR(("vixExporter::reader called in error"), MS::kFailure);
}

/*!
 * @fn MStatus vixExporter::writer(const MFileObject& file, const MString& options, FileAccessMode mode)
 *
 * Called when the plugin is used as an exporter. This plugin converts
 * all or part of the Maya hierarchy into a Vixen scene graph in
 * memory and then writes out a .VIX (Vixen binary) file.
 * It also writes out an ASCII log file
 * which describes which nodes were converted and lists any warnings
 * or errors encountered during the conversion process.
 *
 * Conversion Process passes over the Maya objects several times
 * to make Vixen objects, link them together and convert the
 * data from Maya to Vixen format.
 * @code
 * 1. Scan the Maya objects in breadth-first order making a Maya to Vixen
 *	converter for each one. Store the node converters in a dictionary keyed
 *	by the Maya DAG path. Make a Vixen object for each convertible Maya object.
 *
 *	For each Maya shader encountered
 *
 *	a.  Scan the shader graph in breadth-first order making a Maya to Vixen
 *	    converter for each one. Store the shader converters in a dictionary
 *	    keyed by the Maya object pointer. Make a Vixen object for each
 *	    convertible Maya object.
 *
 *	b.  Scan the Maya shader objects again in breadth-first order, examining
 *	    Maya shader linkage and linking together the Vixen geometry,
 *	    textures and materials.
 *
 *	At the end of the first pass, all Vixen shader objects will be created
 *	and linked together appropriately.
 *
 * 2. Scan the Maya objects again in breadth-first order, examining Maya linkage
 *	and linking together the Vixen nodes in the scene graph.
 *
 * 3. Scan the shader object dictionary (in uncontrollable order) and convert
 *	the data from the Maya object referenced by the converter into Vixen format.
 *	This step does all the shader conversions.
 *
 * 4. Scan the DAG dictionary (in uncontrollable order) and convert
 *	the data from the Maya DAG node referenced by the converter into Vixen format.
 *	This step does all the geometry format conversions.
 *
 * 5. 	Show visual preview window or write Vixen file.
 * @endcode
 *
 * @see vixExporter::exportAll vixExporter::exportSelected
 */
MStatus vixExporter::writer ( const MFileObject& file,
                              const MString& options,
                              FileAccessMode mode )

{
	MStatus			status;   
	MString			mname = file.fullName(), unitName;
	char			namebuf[255];
	char			fname[255];
	Ref<World3D>	world;
	char*			p;

	strcpy(FilePath, file.path().asChar());
	strcpy(FileBase, file.name().asChar());
	p = strrchr(FileBase, '.');
	if (p && (STRCASECMP(p, ".vix") == 0))
		*p = NULL;
	strcpy(fname, mname.asChar());
	strcpy(namebuf, FilePath);
	strcat(namebuf, "/");
	strcat(namebuf, FileBase);
	strcat(namebuf, ".log");
	
	LogFile = new std::ofstream(namebuf, std::ios_base::out);
	if (LogFile->bad())
	    ME_ERROR(("ERROR: The file %s could not be opened for writing", namebuf), MS::kFailure);
	LogFile->precision(3);
	vixen_debug.rdbuf(LogFile->rdbuf()); 
	parseOptions(options);
	world = World3D::Get();
	if (world == NULL)
		world = new World3D();
	world->OnInit();
	Scene = new Vixen::Scene();
	strcpy(namebuf, FileBase);
	strcat(namebuf, ".scene");
	Scene->SetName(namebuf);
	strcpy(namebuf, FileBase);
	strcat(namebuf, ".root");
	Root = new Model();
	Scene->SetModels(Root);
	Root->SetName(namebuf);
	strcpy(namebuf, FileBase);
	strcat(namebuf, ".simroot");
	SimRoot = new Engine();
	SimRoot->SetName(namebuf);
	Scene->SetEngines(SimRoot);

    /*
     * Export the Maya nodes and convert them to Vixen data structures.
     * Write out the .VIX file
     */

	b_LinkPass = false;						// indicate first pass - Make objects
	if ((mode == MPxFileTranslator::kExportAccessMode) ||
		(mode == MPxFileTranslator::kSaveAccessMode))
		exportAll();						// export them all?
	else if (mode == MPxFileTranslator::kExportActiveAccessMode)
		exportSelected();					// export some of them?
	vixen_debug.flush();
	LogFile->flush();
	if (Root != (const Model*) NULL)		// anything exported?
	{
	    if (DoPreview && VIXPreview)		// doing visual preview?
			VIXPreview->showWindow(Scene);	// show the preview window
	    else								// writing a file?
	    {
			meLog(1, "Writing Vixen file %s", fname);
			LogFile->flush();
			writeVixen(fname);
			if (Exporter->LogLevel > 2)
			{
				int printopts = (Exporter->LogLevel > 3) ? SharedObj::PRINT_All : SharedObj::PRINT_Default;
				Scene->Print(*(Exporter->LogFile), printopts);
			}
	    }
	}
	meLog(1, "EXPORT COMPLETE");
	MayaNodes.Empty();							// clear Maya object dicts
	MayaObjects.Empty();
	LogFile->flush();
	delete LogFile;
	LogFile = NULL;
	Root = (Vixen::Model*) NULL;				// get rid of Vixen scene tree
	SimRoot = (Vixen::Engine*) NULL;			// get rid of Vixen engine tree
	Scene = (Vixen::Scene*) NULL;
	world->Stop();
	return MS::kSuccess;
}

/*!
 * @fn void vixExporter::parseOptions(const MString& options)
 * Parse export options and set internal exporter state
 * @code
 *	geometry	output geometry
 *	animation	output animation engine hierarchy
 *	textures	output textures and texture coordinates
 *	width		pixel width of exported RGB textures
 *	height		pixel height of exported RGB textures
 *	preview		visual preview with Vixen instead of saving a file
 * @endcode
 */
void vixExporter::parseOptions(const MString& options)
{
    int i, length = options.length();
    MStringArray optionList;
    MStringArray theOption;
    const char* s1 = options.asChar();

    if (length <= 0)						// nothing to parse?
		return;
    options.split(';', optionList);			// break out all the options.
    for (i = 0; i < length; ++i)
    {
		theOption.clear();
		optionList[i].split('=', theOption);
		MString tmp = theOption[0];
		const char* s2 = tmp.asChar();
		if (theOption[0] == MString("textures") && theOption.length() > 1)
			DoTextures = (theOption[1].asInt() > 0);
		else if (theOption[0] == MString("loglevel") && theOption.length() > 1)
			LogLevel = theOption[1].asInt();
		else if (theOption[0] == MString("animation") && theOption.length() > 1)
			DoAnimation = (theOption[1].asInt() > 0);
		else if (theOption[0] == MString("skinning") && theOption.length() > 1)
			DoSkinning = (theOption[1].asInt() > 0);
		else if (theOption[0] == MString("width") && theOption.length() > 1)
			TexWidth = theOption[1].asInt();
		else if (theOption[0] == MString("height") && theOption.length() > 1)
			TexHeight = theOption[1].asInt();
		else if (theOption[0] == MString("preview") && theOption.length() > 1)
			DoPreview = (theOption[1].asInt() > 0);
 		else if (theOption[0] == MString("duplicateverts") && theOption.length() > 1)
			DupVerts = (theOption[1].asInt() > 0);
		else if (theOption[0] == MString("geometry") && theOption.length() > 1)
			DoGeometry = (theOption[1].asInt() > 0);

   }
}


/*!
 * @fn void vixExporter::initVixen()
 * Called when plugin is first used to do one-time initialization of Vixen.
 *
 * @see vixExporter::creator vixExporter::vixExporter
 */
void vixExporter::initVixen()
{

#ifdef ME_VIXENVIEWER
	World3D* world = World3D::Get();

	if (world == NULL)
	{
		World::Startup();
		world = new ExportWorld();
		world->IncUse();
	}
	Scene = new Scene();
	VIXPreview->makeWindow();
#else
	if (World::Get() == NULL)
		World::Startup();
	Scene = new Vixen::Scene();
#endif
}

bool vixExporter::haveReadMethod () const
{
    return false;
}

bool vixExporter::haveWriteMethod () const
{
    return true;
}

MString vixExporter::defaultExtension () const
{
    return "vix";
}

/*!
 * @fn MPxFileTranslator::MFileKind vixExporter::identifyFile(const MFileObject& fileName, const char* buffer, short n) const
 * @param fileName	name of file to be exported
 * @param buffer	unused
 * @param n		unused
 *
 * Determine if the filename has the correct extension for this exporter.
 * The Vixen exporter currently only recognizes ".vix" as a valid extension
 *
 * @return kCouldBeMyFileType if file extension is ".vix", else kNotMyFileType
 */
MPxFileTranslator::MFileKind vixExporter::identifyFile(const MFileObject& fileName, const char*, short) const
{
    const char* name = fileName.name().asChar();
    size_t	   nameLength = strlen(name);
    
    if (nameLength <= 4)
		return kNotMyFileType;
    name += nameLength - 4;
    if (STRCASECMP(name, ".vix") == 0)
		return kIsMyFileType ;
    return kNotMyFileType;
}

/*!
 * @fn MStatus initializePlugin( MObject obj )
 *
 * Initializes the exporter plugin and registers it with Maya.
 * This function connects the plugin user interface and options with the C++ code.
 */
MStatus initializePlugin( MObject obj )
{
	MFnPlugin plugin(obj, "Visual Experience Engine", "1.0", "Any");

	return plugin.registerFileTranslator(
		"VIXexport",
		"none",
		vixExporter::creator,
		(char *) vixOptionScript,
		(char *) vixDefaultOptions );                                        
}

/*!
 * @fn MStatus uninitializePlugin( MObject obj )
 *
 * Unregisters the plugin from Maya, making it no longer available to the user.
 * TODO: We should clean up all Vixen data structures here.
 */
MStatus uninitializePlugin( MObject obj )
{
    MFnPlugin plugin( obj );

    Exporter = NULL;
    return plugin.deregisterFileTranslator( "VIXexport" );
}


/*!
 * @fn MStatus vixExporter::exportSelected( )
 *
 * Called to export the currently selected nodes as a Vixen binary file.
 * Traverses the Maya selection list and exports the hierarchies below
 * each selected node.
 *
 * Conversion Process passes over the Maya objects several times
 * to make Vixen objects, link them together and convert the
 * data from Maya to Vixen format.
 * @code
 *
 * PASS 1: For each Maya object selected
 *
 *	a.  Scan descendants of the selected object in breadth-first order
 *	    making a Maya to Vixen converter and a Vixen object for each.
 *	    Node converters are stored in the "MayaNodes" dictionary indexed
 *	    by DAG path.
 *
 *	b.  For each Maya shader encountered
 *	    -   Scan the shader graph in breadth-first order, making a Maya to
 *			Vixen converter and a Vixen object for each one.
 *	    -   Scan the Maya shader objects again in breadth-first order and
 *			link them together appropriately.
 *
 *	In the first pass b_LinkPass is false, causing a recursive call to make
 *	the second pass over selected objects and children.
 *
 * PASS 2: For each Maya object selected
 *
 *	   Scan descendants of the selected object in breadth-first order
 *	   examining Maya linkage and linking together the Vixen nodes in the scene graph.
 *
 * PASS 3: Convert accumulated scene graph nodes (see vixExporter::convertAll)
 * @endcode
 *
 * @see vixExporter::exportAll
 */
MStatus vixExporter::exportSelected( )
{
    MStatus	status;
    MString	filename;
    MDagPath	dagPath;

    MGlobal::getActiveSelectionList(Selected);
    MItSelectionList seliter(Selected);		// iterator for selected objects

	b_SelectedOnly = true;
    if (seliter.isDone())
		ME_ERROR(("ERROR: Nothing is selected"), MS::kFailure);
    meLog(1, "%s SELECTED SCENE OBJECTS", b_LinkPass ? "LINKING" : "SCANNING");
/*
 * We will need to interate over a selected node's heirarchy 
 * in the case where shapes are grouped, and the group is selected.
 */
    for ( ; !seliter.isDone(); seliter.next())	// for each selected node
    {	 
		MObject  dagNode;

		status = seliter.getDagPath(dagPath);
		if (!status)
			ME_ERROR(("ERROR: Failure getting DAG path from selection"), MS::kFailure);
		if (!seliter.getDependNode(dagNode))
			ME_ERROR(("ERROR: Failure getting DAG node from selection"), MS::kFailure);

		MItDag dagiter(MItDag::kBreadthFirst);
		dagiter.reset(dagPath, MItDag::kBreadthFirst);
		for ( ; !dagiter.isDone(); dagiter.next() )
		{
			status = dagiter.getPath(dagPath);

			if (!status)
				ME_ERROR(("ERROR: Failure getting DAG path"), MS::kFailure);
			if (b_LinkPass)
				linkNode(dagPath);
			else
				makeNode(dagPath);
		}
    }
    if (b_LinkPass)
		return convertAll();
    b_LinkPass = true;
    return exportSelected();
}

/*!
 * @fn MStatus vixExporter::exportAll( )
 *
 * Called to export the entire Maya hierarchy as a Vixen binary file.
 * Conversion Process passes over the Maya objects several times
 * to make Vixen objects, link them together and convert the
 * data from Maya to Vixen format.
 * @code
 *
 * PASS 1: Scan Maya DAG in breadth-first order, making a Maya to Vixen
 *	   converter and a Vixen object for each.
 *	   Node converters are stored in the "MayaNodes" dictionary indexed by DAG path.
 *
 *	    For each Maya shader encountered
 *	    -   Scan the shader graph in breadth-first order, making a Maya to
 *			Vixen converter and a Vixen object for each one.
 *	    -   Scan the Maya shader objects again in breadth-first order and
 *			link them together appropriately.
 *
 *	   In the first pass b_LinkPass is false, causing a recursive call to make the second pass.
 *
 * PASS 2: Scan Maya DAG in breadth-first order, examining Maya linkage and
 *	   linking together the Vixen nodes in the scene graph.
 *
 * PASS 3: Convert accumulated scene graph nodes (see vixExporter::convertAll)
 * @endcode
 *
 * @see vixExporter::exportSelected
 */
MStatus vixExporter::exportAll( )
{
    MStatus status = MS::kSuccess;
    MItDag dagiter(MItDag::kBreadthFirst, MFn::kInvalid, &status);

	b_SelectedOnly = false;
    meLog(1, "%s SCENE GRAPH", b_LinkPass ? "LINKING" : "SCANNING");
    if (MS::kSuccess != status)
		ME_ERROR(("ERROR: Failure in DAG iterator setup"), MS::kFailure);
    for ( ; !dagiter.isDone(); dagiter.next() )
    {
		MDagPath dagPath;
		status = dagiter.getPath(dagPath);

		if (MS::kSuccess != status)
			ME_ERROR(("ERROR: Failure getting DAG path"), MS::kFailure);
		if (b_LinkPass)
			linkNode(dagPath);
		else
			makeNode(dagPath);
    }
    if (b_LinkPass)
		return convertAll();
    b_LinkPass = true;
    return exportAll();
}

/*!
 * @fn meConvObj* vixExporter::makeNode(MDagPath& dagPath)
 * @param dagPath	Maya DAG path of node to make
 *
 * Makes the converter class and the Vixen node for the Maya
 * node referenced by the given DAG path. If the Maya node cannot
 * be converted to Vixen format, it is ignored. Otherwise,
 * the DAG path is saved in an internal dictionary that maps
 * DAG paths to node converters
 *
 * @return converter made for Maya object, NULL on error
 *
 * @see meConvObj vixExporter::convertAll vixExporter::exportAll vixExporter::exportSelected
 */
meConvObj* vixExporter::makeNode(MDagPath& dagPath)
{
    MStatus		status1, status2;
    MObject		nodeobj = dagPath.node(&status1);
    MString		dagName = dagPath.fullPathName(&status2);
    const char*	c = dagName.asChar();

    if (!nodeobj.hasFn(MFn::kDagNode))
		ME_ERROR(("ERROR: Failure in MFnDagNode constructor for %s", c), NULL);
	ConvRef		cvref = MayaNodes[dagPath];
	meConvObj*	converter = cvref;
	if (converter)
		return converter;
	if (b_SelectedOnly && !Selected.hasItem(dagPath))
		return NULL;
	converter = meConvNode::MakeConverter(dagPath);
	if (converter)
	{
		SharedObj* node = converter->Make();	// make the Vixen node
		if (node)								// if successful, keep this converter
		{
			converter->DoConvert = true;
			MayaNodes.Set(dagPath, converter);
			if (converter->DoScan > 0)				// make converters for upstream nodes
			{										// not in the DAG
				converter->DoScan = -1;
				scanInputs(converter->GetMayaObj());
			}
		}
	}
    LogFile->flush();
    return converter;
}

/*!
 * @fn meConvObj* vixExporter::makeOne(MObject mayaObj)
 * @param mayaObj	Maya object to make a converter for
 *
 * Makes the converter class and the Vixen object for the input Maya object.
 * If the Maya node cannot be converted to Vixen format, it is ignored.
 * Otherwise, the object reference is saved in an internal dictionary that maps
 * Maya objects to node converters
 *
 * @return converter made for Maya object, NULL on error
 *
 * @see meConvObj vixExporter::convertAll vixExporter::exportAll vixExporter::exportSelected
 */
meConvObj* vixExporter::makeOne(MObject& mayaObj)
{
 	Ref<meConvObj>	conv;
 
    if (mayaObj.hasFn(MFn::kDagNode))		// is a DAG node?
	{
		MFnDagNode	dagnode(mayaObj);		// get Maya node interface
		MDagPath	dagPath;

		dagnode.getPath(dagPath);
		return makeNode(dagPath);
	}
	if (mayaObj.hasFn(MFn::kDependencyNode))
	{
		MFnDependencyNode	node(mayaObj);

		conv = MayaObjects[mayaObj];
		if (!conv.IsNull())
			return conv;
		conv = meConvObj::MakeConverter(mayaObj);
		if (!conv.IsNull())
			if (conv->Make())
			{
				conv->DoConvert = true;
				MayaObjects.Set(mayaObj, conv);
			}
			else
				conv = NULL;
		LogFile->flush();
	}
    return conv;
}


/*!
 * @fn meConvObj* vixExporter::linkNode(MDagPath& dagPath)
 * @param dagPath		Maya DAG path of node to make
 * @param convparent	converter that should be the parent of this one
 *						default is NULL
 *
 * Links the Vixen node held by this converter into the
 * Vixen hierarchy. The "DoLink" flag must be true
 * for this converter for linkage to occur.
 * If the parent converter is provided, the Vixen object associated
 * with it will be linked to the Vixen object for this converter.
 * If it is NULL, linkage will be inferred from the Maya hierarchy.
 *
 * @return converter for the node, NULL on error
 *
 * @see meConvObj vixExporter::convertAll meConvObj::Link
 */
meConvObj* vixExporter::linkNode(MDagPath& dagPath, meConvObj* convparent)
{
    MObject		nodeobj = dagPath.node();
	ConvRef		cvref  = MayaNodes[dagPath];
    meConvObj*	converter = cvref;

	if (converter == NULL)
	{
		cvref = MayaObjects[nodeobj];
		converter = cvref;
	}
   if (converter && converter->DoLink)		// OK to link?
    {
		if (converter->Link() >= 0)			// link succeeded?
			converter->DoConvert = true;	// mark as OK to convert
		converter->DoLink = false;			// prevent another link attempt
		if (converter->DoScan < 0)			// link converters for upstream nodes
		{									// not in the DAG
			converter->DoScan = 0;
			scanInputs(converter->GetMayaObj());
		}

		return converter;
    }
    return NULL;
}

meConvObj* vixExporter::linkOne(MObject& nodeobj, meConvObj* convparent)
{
	MStatus		status;
    MFnDagNode	dagnode(nodeobj, &status);	// get Maya node interface
	ConvRef		cvref;
 	meConvObj*	conv;
 
    if (status)								// is a DAG node?
		return linkNode(dagnode.dagPath(), convparent);
    cvref = MayaObjects[nodeobj];
	conv = cvref;
    if (conv && conv->DoLink)			// OK to link?
    {
		if (conv->Link(convparent) >= 0)// link succeeded?
			conv->DoConvert = true;		// mark as OK to convert
		conv->DoLink = false;			// prevent another link attempt
		return conv;
    }
    return NULL;
}

/*!
 * @fn MStatus vixExporter::convertAll()
 *
 * Converts the Maya nodes referenced in the the DAG dictionary into
 * Vixen nodes. Upon return, PFRoot references the complete
 * Vixen hierarchy made from the Maya nodes. This hierarchy can
 * be saved in a binary .VIX file
 *
 * Conversion process is as follows:
 * 1. Make Vixen nodes for all Maya nodes to be converted (done by caller)
 * 2. Link together shader nodes hierarchy for each state (done by caller)
 * 3. Link together scene graph hierarchy (done by caller)
 * 4. Convert all engine and shader nodes
 * 5. Convert all geometry nodes
 * 6. Sample animations
 *
 * @see vixExporter::makeOne vixExporter::exportAll vixExporter::exportSelected
 */
MStatus vixExporter::convertAll()
{
    MStatus status = MS::kSuccess;
    ConvRef* cref;

    b_LinkPass = false;
    setCamera();
	/*
	 * Convert the Shader network of objects
	 */
    meLog(1, "\nCONVERTING MATERIALS AND TEXTURES");
    meObjDict::Iter siter2(&MayaObjects);
    while (cref = siter2.Next())
    {
		meConvObj* converter = *cref;
		meConvState* stateconv = dynamic_cast<meConvState*>( converter );
		if (stateconv && stateconv->DoConvert)
		{
			if (stateconv->Convert() < 0)
				status = MS::kFailure;
			stateconv->DoConvert = false;
		}
    }
    LogFile->flush();
/*
 * Convert Nodes and Geometry next. During this step we may coalesce materials,
 * textures and shaders which are the same in Vixen but are not
 * the same Maya objects
 */
    meLog(1, "\nCONVERTING GEOMETRY IN SCENE GRAPH");
    meDagDict::Iter hiter2(&MayaNodes);
    ConvRef* nref;

    while (nref = hiter2.Next())
    {
		meConvObj* converter = *nref;
		if (converter && converter->DoConvert)
		{
			if (converter->Convert() < 0)
				status = MS::kFailure;
			converter->DoConvert = false;
		}
    }
	/*
	 * Convert the Engine objects
	 */
    meLog(1, "\nCONVERTING ENGINES");
    siter2.Reset(&MayaObjects);
    while (nref = siter2.Next())
    {
		meConvObj*	converter = *nref;
		meConvEng*	engconv = dynamic_cast<meConvEng*>( converter );
		if (engconv && engconv->DoConvert)
		{
			if (engconv->Convert() < 0)
				status = MS::kFailure;
			engconv->DoConvert = false;
		}
    }
    LogFile->flush();
	if (Exporter->DoAnimation)
		animateAll();
    return status;
}

/*!
 * @fn MStatus vixExporter::animateAll()
 *
 * Inspects the Maya DAG nodes for animation curves and evaluates the
 * transform of each node over the animation playback period.
 * Only rotations and positions are exported as Vixen KeyFramer objects.
 * *
 * @see vixExporter::convertAll vixExporter::exportAll vixExporter::exportSelected
 */
MStatus vixExporter::animateAll()
{
    MStatus status = MS::kSuccess;
//	MTime	Start = MAnimControl::animationStartTime();
//	MTime	End = MAnimControl::animationEndTime();
	MTime	Start = MAnimControl::minTime();
	MTime	End = MAnimControl::maxTime();
	double	tstart = Start.as(MTime::kSeconds);
	double	dur = End.as(MTime::kSeconds);
	double	timeinc = MTime(1, MTime::uiUnit()).as(MTime::kSeconds);
	int		nframes = 0;

	meLog(1, "\nCOLLECTING ANIMATION DATA from %f to %f", tstart, dur);
	dur -= tstart;
	for (double t = 0; t <= dur; t += timeinc)
	{
		meDagDict::Iter hiter2(&MayaNodes);
		ConvRef*	nref;
		MTime		curtime = MTime(t + tstart, MTime::kSeconds);

		MAnimControl::setCurrentTime(curtime);
		while (nref = hiter2.Next())
		{
			meConvObj* converter = *nref;
			meConvJoint* jconv = dynamic_cast<meConvJoint*>( converter );
			meConvNode* nconv = dynamic_cast<meConvNode*>( converter );
			Interpolator* position = NULL;
			Interpolator* rotation = NULL;
			Interpolator::WQuat rot;
			Vec3	pos;
			Matrix	mtx;

			if (jconv && jconv->HasAnimation)
			{
				position = jconv->Position;
				rotation = jconv->Rotation;
				jconv->GetMatrix(mtx);
			}
			else if (nconv && nconv->Animation)
			{
				Transformer* xform = nconv->Animation;
				Interpolator* eng;
				nconv->GetMatrix(mtx);
				Engine::Iter iter(xform, Group::CHILDREN);
				while (eng = (Interpolator*) iter.Next())
					if (eng->IsClass(VX_Interpolator))
						if (eng->GetValSize() == 3)
							position = eng;
						else rotation = eng;
			}
			mtx.GetTranslation(pos);
			rot.Set(mtx);
			rot.weight = 1.0f;
			if (position != NULL)
				position->AddKey(t, pos);
			if (rotation != NULL)
				rotation->AddKey(t, rot);
		}
		++nframes;
	}
    LogFile->flush();
    return status;
}

/*!
 * @fn bool vixExporter::scanInputs(MObject& mayaObj)
 * Scan backwards thru the dependency graph for all the Maya objects which
 * modify the input object. If any of them correspond to a Vixen engine
 * (skinning, skeletal animation, deformation), this function makes
 * the appropriate Vixen engine and puts it into the simulation tree at the root.
 * Those which correspond to shader network nodes are converted to the corresponding
 * Vixen materials and textures.
 *
 * @return  true if any Vixen objects are generated, else false
 *
 * @see meConvEng::MakeConverter vixExporter::convertAll meConvObj::Link
 */
bool vixExporter::scanInputs(MObject& mayaObj)
{
    MItDependencyGraph iter(mayaObj, MFn::kInvalid, MItDependencyGraph::kUpstream,
			    MItDependencyGraph::kBreadthFirst, MItDependencyGraph::kNodeLevel);

/*
 * Make converters for all Maya nodes which modify this object and
 * have counterparts in Vixen (can be expressed as Vixen engines).
 * Graph is scanned breadth first so that nodes at a higher level
 * in the tree have converters created before their children.
 */
    iter.disablePruningOnFilter();
    for ( ; !iter.isDone(); iter.next() )
    {
		MObject& obj(iter.thisNode());
		MFnDependencyNode node(obj);

		if (obj == mayaObj)				// prevent recursion
			continue;
		if (!obj.hasFn(MFn::kDependencyNode))
			ME_ERROR(("ERROR: node has invalid type"), false);
		meLog(4, "ScanInputs: %s %s", node.name().asChar(), obj.apiTypeStr());
		if (b_LinkPass)
			linkOne(obj);
		else
			makeOne(obj);
    }
	return true;
}

/*!
 * @fn MStatus vixExporter::setCamera()
 *
 * Set the matrix on the Vixen camera so the scene is viewed thru the
 * Maya camera. This involves taking the inverse of the total transform
 * from the root to the camera and rotating the scene about X to flip
 * the coordinate systems to be consistent with Vixen.
 *
 * @see vixExporter::convertAll
 */
void vixExporter::setCamera()
{
    M3dView			view = M3dView::active3dView();
    MDagPath		campath;
 	Camera*			cam = Scene->GetCamera();

    if (view.getCamera(campath))
    {
		ConvRef cvref = Exporter->MayaNodes[campath];
		meConvObj* conv = cvref;
		if (conv == NULL)
		{
			campath.pop();
			cvref = Exporter->MayaNodes[campath];
			conv = cvref;
			if (conv == NULL)
				return;
			cam = (Camera*) conv->GetVXObj();
			VX_ASSERT(cam && cam->IsClass(VX_Camera));
			Scene->SetCamera(cam);
			return;
		}
    }
}

/*!
 * @fn bool vixExporter::writeVixen(const char* fname)
 * @param fname	full name of file to contain exported Vixen scene
 *
 * Writes the Vixen scene constructed in memory into a
 * binary file on disk.
 */
bool vixExporter::writeVixen(const char* fname)
{
	FileMessenger		vixout;				// create messenger to serialized scene
	Core::FileStream	fstream;			// create stream to output the serialized scene
	vixout.SetOutStream(&fstream);
	if (vixout.Open(fname, Messenger::OPEN_WRITE))	// open binary output file
	{
		pruneScene(Scene);					// prune unwanted Vixen objects
		Scene->Save(vixout, 0);				// save Vixen hierarchy to the file
		vixout.Close();
		return true;
	}
	return false;
}

/*!
 * @fn void vixExporter::pruneScene(Scene* scene)
 * @param scene	scene to prune
 *
 * Prunes the scene based on file export options settings.
 * If either animation or skinning export is not selected,
 * no engines will be exported. If geometry export is not selected,
 * no models will be exported.
 *
 * Models which are directly under the root with no children
 * are pruned (Cameras and Lights are not pruned).
 * Transformers (joints) which are directly under the root which
 * no children and not target are pruned.
 *
 */
void vixExporter::pruneScene(Vixen::Scene* scene)
{
	if (DoAnimation || DoSkinning)		// exporting animation?
	{
		Engine*	simroot = scene->GetEngines();

		if (simroot)					// get rid of empty transformers with no target
		{
			Engine*	tmp = simroot->First();

			while (tmp)
			{
				Engine* next = tmp->Next();

				if (!tmp->IsParent() &&
					(tmp->ClassID() == VX_Transformer) &&
					(tmp->GetTarget() == NULL))
					tmp->Remove(Group::UNLINK_FREE);
				tmp = next;
			}
		}
	}
	else							// nuke the engines if not exporting animation
		scene->SetEngines(NULL);
	if (DoGeometry)					// exporting geometry?
	{
		Model*	root = scene->GetModels();
		if (root)					// get rid of empty models with no children
		{
			Model*	tmp = root->First();

			while (tmp)
			{
				Model* next = tmp->Next();

				if (!tmp->IsParent() && (tmp->ClassID() == VX_Model))
					tmp->Remove(Group::UNLINK_FREE);
				tmp = next;
			}
		}
	}
	else					// nuke the models if not exporting geometry
		scene->SetModels(NULL);
}
