
#include "vxwxwin.h"

#if defined(_WIN32)
namespace Vixen {
#endif

class wxViewApp : public Viewer<World3D>
{
public:
	wxViewApp();
};

wxViewApp::wxViewApp() : Viewer<World3D>()
{
	DoAsyncLoad = false;
}

#if defined(_WIN32)
VIXEN_MAKEWORLD(Vixen::wxViewApp);
}
#else
VIXEN_MAKEWORLD(wxViewApp);
#endif