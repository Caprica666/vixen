#include "mepreview.h"

#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MEvent.h>

IMPORT mePreview* VIXPreview;

MStatus	initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, "Intel", "1.0", "Any");
    status = plugin.registerContextCommand("vixViewer", vixViewCmd::creator);
    return status;
}

MStatus	uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);
    status = plugin.deregisterContextCommand("vixViewer");
    return status;
}

/*!
 * @fn vixViewer::vixViewer()
 * Constructor for Maya visual preview plugin.
 * mePreview::ViewIndex contains the view index of the Maya preview window.
 * We use it to get the Maya view to take over for preview.
 *
 * @see mePreview::makeWindow
 */
vixViewer::vixViewer()
{
    m_Window = NULL;
    b_Exit = false;
    m_IsPressed = false;
	m_World = new Viewer<World3D>;
	ViewIndex = 0;	// TODO: figure out what this should be
    M3dView::get3dView(ViewIndex, m_VixView);
	m_World->OnInit();
}

/*!
 * @fn vixViewer::~vixViewer()
 * Destructor for visual preview plugin.
 *
 * @see vixViewer::toolOnSetup
 */
vixViewer::~vixViewer()
{
	m_World->OnExit();
}

/*!
 * @fn void vixViewer::MakeScene(Scene* scene)
 * Set the Vixen scene graph to display.
 * This routine assumes the Vixen window has already been set up.
 */
void vixViewer::SetScene(Scene* scene)
{
	Transformer* navroot = new Transformer();

	navroot->SetName("viewer.camera.xform");
	m_Scene->SetEngines(navroot);
	m_Navigator = new Flyer;
	navroot->Append(m_Navigator);
	m_Scene->SetModels(scene->GetModels());
	if (scene->GetEngines())
		navroot->Append(scene->GetEngines());
 }

/*!
 * @fn void vixViewer::toolOnSetup(MEvent& event)
 * Callback invoked by Maya when this viewer tool is chosen.
 * This is indirectly called by mePreview::showWindow when
 * each time a VIX file is exported.
 *
 * @see vixViewer::doExit
 */
void vixViewer::toolOnSetup(MEvent& event)
{
    MPxContext::toolOnSetup(event);
    if (PerfPreview == NULL)
		return;
    b_Exit = false;
    m_MayaView = M3dView::active3dView();
    M3dView::get3dView(ViewIndex, m_VixView);
    setHelpString("Press ENTER in the main window to end preview");
    m_VixView.beginGL();
    glPushAttrib(GL_ALL_ATTRIB_BITS);	// save Maya GL state, popped in doExit
	SetScene(VixPreview->Scene);		// set Vixen scene
    doFrame();
    m_VixView.endGL();
}


/*!
 * @fn void vixViewer::doExit()
 * Called when the ENTER key is pressed within the Maya window to shut down
 * Vixen display. This routine does NOT close the Vixen window or
 * release the GL context. It just hides the window and stops responding to
 * Maya input events.
 */
void vixViewer::doExit()
{
    if (b_Exit || (VixPreview == NULL))
		return;
    b_Exit = true;
    m_VixView.beginGL();
	m_Scene->Exit();
	m_Scene = NULL;
    m_VixView.endGL();
    toggleWindow();
    glPopAttrib();					// pushed in toolOnSetup
    m_MayaView.refresh(false, true);
}


/*!
 * @fn bool vixViewer::initWindow()
 * Creates a Vixen scene and attaches it to a Maya window.
 * This should be called once at startup after the Maya window is
 * created and never again. It extracts the GL context and window
 * handle information from the Maya window and associates it with a
 * new Vixen scene.
 */
bool vixViewer::initWindow()
{
    Scene*		scene = new GLScene();
	DeviceInfo*	devinfo = scene->GetDevInfo();
    M3dWindow	win = m_VixView.window();
#ifdef _WIN32
    HGLRC glctx = m_VixView.display();
#else
    GLXContext glctx = m_VixView.glxContext();
#endif

	devinfo->WinHandle = NULL;
	devinfo->Device = glctx;
	devinfo->DC = NULL;
	m_Scene = scene;
	scene->SetOptions(Scene::CLEARALL | Scene::STATESORT | Scene::DOUBLEBUFFER);
    return true;
}

/*!
 * @fn void vixViewer::toggleWindow()
 * Toggles the visibility of the Maya window used to display
 * the Vixen scene graph. This is called to hide the Vixen display.
 *
 * @see vixViewer vixViewer::showWindow
 */
void vixViewer::toggleWindow()
{
    MGlobal::executeCommand("toggleWindowVisibility vixWin;", false, false);
}

/*!
 * @fn void vixViewer::doMouse(MEvent& event)
 * Processes a Maya mouse event and passes it to the Vixen navigator
 * controlling the camera.
 */
void vixViewer::doMouse(MEvent& me)
{
    MouseEvent*	ev = new MouseEvent();
    short xpos, ypos;

	ev->MouseFlags = 0;
    if (m_IsPressed)
    {
		if (me.mouseButton() & MEvent::kLeftMouse)
			ev->MouseFlags |= MouseEvent::LEFT;
		if (me.mouseButton() & MEvent::kMiddleMouse)
			ev->MouseFlags |= MouseEvent::MIDDLE;
 		if (me.mouseButton() & MEvent::kRightMouse)
			ev->MouseFlags |= MouseEvent::RIGHT;
   }
    if (me.modifiers() & MEvent::shiftKey)
		ev->MouseFlags |= MouseEvent::SHIFT;
    if (me.modifiers() & MEvent::controlKey)
		ev->MouseFlags |= MouseEvent::CONTROL;
    me.getPosition(xpos, ypos);
	ev->MousePos.Set(xpos, ypos);
    m_Navigator->OnEvent(ev);
	delete ev;
}

/*!
 * @fn void vixViewer::doPress(MEvent& event)
 * Processes a Maya mouse down event.
 *
 * @see vixViewer::doRelease vixViewer::doMouse
 */
MStatus	  vixViewer::doPress(MEvent& event)
{
    m_IsPressed = true;
    doMouse(event);
    doFrame();
    return MStatus::kSuccess;
}

/*!
 * @fn void vixViewer::doRelease(MEvent& event)
 * Processes a Maya mouse up event. It is possible to get this event
 * WITHOUT getting a mouse down event first (not sure why).
 *
 * @see vixViewer::doRelease vixViewer::doMouse
 */
MStatus	  vixViewer::doRelease(MEvent& event)
{
    if (!m_IsPressed)
		return MStatus::kSuccess;
    m_IsPressed = false;
    doMouse(event);
    doFrame();
    return MStatus::kSuccess;
}

/*!
 * @fn void vixViewer::doDrag(MEvent& event)
 * Processes a Maya mouse drag event. It is possible to get this event
 * WITHOUT getting a mouse down event first (not sure why).
 *
 * @see vixViewer::doRelease vixViewer::doMouse
 */
MStatus	  vixViewer::doDrag(MEvent& event)
{
    if (m_IsPressed)
    {
		doMouse(event);
		doFrame();
    }
    return MStatus::kSuccess;
}

/*!
 * @fn void vixViewer::doFrame()
 * Display the next Vixen frame. This routine is called whenever the
 * mouse is moved within the Vixen display window
 *
 * @see vixViewer::initWindow vixViewer::toolOnSetup
 */
void vixViewer::doFrame()
{
    if (!b_Exit)
    {
		m_VixView.beginGL();
		m_Scene->DoFrame();
		m_VixMView.endGL();
    }
}

