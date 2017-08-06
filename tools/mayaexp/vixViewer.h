#pragma once

#include <maya/MPxContext.h>
#include <maya/MPxContextCommand.h>
#include <maya/M3dView.h>



/*!
 * @class vixViewer
 * Maya plugin which displays a Vixen scene within a Maya window.
 * We derive from MPxContext so we can get the input keyboard and mouse
 * events from Maya.
 *
 * This plugin communicates with the VIX export plugin which generates
 * the Vixen scene to display. They share a common block of information
 * encapsulated in the mePreview class.
 *
 * @see vixExport mePreview
 */
class vixViewer : public MPxContext
{
public:
    vixViewer();
    ~vixViewer();

    void	  toolOnSetup(MEvent& event);
    void	  completeAction()	{ doExit(); }
    void	  abortAction()		{ doExit(); }
    MStatus	  doPress(MEvent& event);
    MStatus	  doRelease(MEvent& event);
    MStatus	  doDrag(MEvent& event);

protected:
    void	  doMouse(MEvent& event);
    void	  doFrame();
    void	  doExit();
    void	  toggleWindow();

    bool			b_Exit;
    bool			b_DoPop;
    bool			m_IsPressed;
	int				m_ViewIndex;
	Ref<World>		m_World;
    Ref<Scene>		m_Scene;
	Ref<Navigator>	m_Navigator;
    M3dView			m_VixView;
    M3dView			m_MayaView;
};

class vixViewCmd : public MPxContextCommand
{
public:
    vixViewCmd() : MPxContextCommand()	{ }
    MPxContext*	 makeObj()		{ return new vixViewer(); }
    static void* creator()		{ return new vixViewCmd(); }
};

#endif
