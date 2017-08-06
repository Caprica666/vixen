#pragma once

#ifdef WIN32
#define EXPORT  __declspec(dllexport)
#define IMPORT  __declspec(dllimport)

#else
#define EXPORT
#define IMPORT extern
#endif
#include <maya/MTypes.h>
#include "vixen.h"
#include <maya/MString.h>
#include <maya/MGlobal.h>
#include <maya/M3dView.h>

/*!
 * @class mePreview
 * Encapsulates information common between export and viewer plugins.
 * The vixExport plugin has the capability to display the Vixen scene
 * in a Maya window. To control the Vixen display, we use the vixViewer
 * plugin so we can capture input events on a Maya window. This class is
 * initialized by the exporter plugin and used by the viewer plugin
 * to access the Vixen scene graph.
 */
class mePreview
{
public:
    mePreview() { ViewIndex = 0; HasWindow = false; HasPlugin = false; }

    bool	showWindow(Vixen::Scene* scene);
    bool	toggleWindow();
    bool	makeWindow();

    int					ViewIndex;
    bool				HasWindow;
    bool				HasPlugin;
    Ref<Vixen::Scene>	Scene;
};

