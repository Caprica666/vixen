#include "vix_all.h"

using namespace Vixen;

/*!
 * @fn bool mePreview::makeWindow()
 * Creates the Maya window used for Vixen visual preview.
 */
bool mePreview::makeWindow()
{
    MString name;

    if (HasWindow)
		return true;
    ViewIndex = M3dView::numberOf3dViews();
    MGlobal::executeCommand("loadPlugin vixViewer;", name, false, false);
    if (!MGlobal::executeCommand("window -retain -w 300 -h 300 -title \"Vixen Viewer\" vixWin; frameLayout -label \"Press ENTER in the main window to end preview\" vixFrame;", false, false))
		return false;
    HasWindow = true;
    return true;
}

/*!
 * @fn bool mePreview::showWindow(Scene* scene)
 * Called when visual preview is initiated to show the preview window and switch
 * to the visual preview tool in Maya.
 */
bool mePreview::showWindow(Vixen::Scene* scene)
{
    if (!HasWindow)
		return false;
    if (!HasPlugin)
    {
		if (!MGlobal::executeCommand("modelEditor -parent vixFrame vixEditor; vixViewer vixView;", false, false))
			return false; 
		if (!MGlobal::executeCommand("showWindow vixWin;", false, false))
			return false;
    }
    else toggleWindow();
    HasPlugin = true;
    Scene = scene;
    if (!MGlobal::executeCommand("modelEditor -edit -uci false -allObjects 0 -grid 0 vixEditor;", false, false))
        return false;
    if (!MGlobal::executeCommand("setFocus vixFrame; setToolTo vixView;", false, false))
        return false;
    return true;
}

/*!
 * @fn bool mePreview::toggleWindow()
 * Called when visual preview is ended to hide the preview window.
 */
bool mePreview::toggleWindow()
{
    if (MGlobal::executeCommand("toggleWindowVisibility vixWin;", false, false))
        return true;
    else
		return false;
}

