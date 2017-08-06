#include "vxmfc.h"

namespace Vixen {
namespace MFC {

IMPLEMENT_DYNCREATE(Doc, CDocument)

BEGIN_MESSAGE_MAP(Doc, CDocument)
	//{{AFX_MSG_MAP(Doc)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

Doc::Doc() { }

/****
 *
 * OnFileOpen
 * Displays the file open dialog and allows the user to choose
 * the 3D scene file to open. The file is read in and the root node
 * of its scene graph becomes the current hierarchy
 * (replacing whatever was there).
 *
 ****/
void Doc::OnFileOpen() 
{
	CFileDialog openDialog(TRUE, TEXT("*.vix"), TEXT("test.vix"));
	openDialog.m_ofn.lpstrFilter = TEXT("Scene Description Files (*.vix)\0*.vix\0Havok Tag Files (*.hkt)\0*.hkt\0All Files (*.*)\0*.*\0\0");
	openDialog.m_ofn.lpstrInitialDir = TEXT("data");
	if (openDialog.DoModal() == IDOK)
	{
		m_FileName = openDialog.GetPathName();
		m_FileRoot = openDialog.GetFileName();
		m_FileRoot = m_FileRoot.Left(m_FileRoot.ReverseFind('.'));
		OnNewDocument();
	}
}

/****
 *
 * Name: OnNewDocument
 *
 * Description:
 * Opens the 3D scene file whose name is stored in m_FileName.
 * and adds the root node (file.root) to the scene as
 * the current scene graph (replacing and freeing the
 * one that was there)
 *
 ****/
BOOL Doc::OnNewDocument()
{
	Demo*			pApp = Demo::Get();
	const TCHAR*	fname = pApp->GetFileName();
	Messenger*	mess = pApp->GetMessenger();

	if (m_FileName.GetLength() > 0)
		pApp->SetFileName(fname = m_FileName);
	else if (fname == NULL)
		return TRUE;
	if (mess)
		mess->Observe(pApp, Event::LOAD_SCENE, NULL);	
	pApp->LoadAsync(fname, GetMainScene());
	SetPathName(fname);		// sets title bar, adds to MRU list
	return TRUE;
}

void Doc::Serialize(CArchive& ar)
{
	Demo*	app = Demo::Get();
	TCHAR* fname = (TCHAR*) (const TCHAR*) ar.m_strFileName;
	if (ar.IsStoring())
	{
		if (!OnSaveDocument(fname))
			AfxThrowArchiveException(CArchiveException::genericException, fname);
	}
/*
 * Unload previous scene file, if any
 * Tell current scene to observe load events
 * Establish new filename and load the file
 */
	else
	{
		app->Unload(app->GetFileName());
		app->GetMessenger()->Observe(app, Event::LOAD_SCENE, NULL);	
		app->SetFileName(fname);
		app->LoadAsync(fname, app->GetScene());
	}
}


#ifdef _DEBUG
void Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


/****
 *
 * OnFileSaveAs
 * Displays the file save dialog and allows the user to choose
 * the scene file to save. Currently only ".jpg" is allowed as the extension
 * and it saves the framebuffer to a JPG
 *
 ****/
void Doc::OnFileSaveAs() 
{
	OPENFILENAME ofn;
	TCHAR		buf[200];
	Scene*		scene = GetMainScene();
	bool		opened = false;

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFilter = TEXT("Vixen Scenes (*.vix)\0*.vix\0All Files (*.*)\0*.*\0\0");
	ofn.lpstrDefExt = TEXT("vix");
	ofn.lpstrInitialDir = TEXT("..\\Data");
	ofn.lpstrFile = buf;
	ofn.Flags = OFN_CREATEPROMPT;
	ofn.nMaxFile =200;
	*buf = 0;
	scene->Suspend();
	opened = GetSaveFileName(&ofn) != 0;
	scene->Resume();
	if (opened)
		OnSaveDocument(ofn.lpstrFile);
}

BOOL Doc::OnSaveDocument(LPCTSTR fname)
{
	Demo*		world = Demo::Get();
	size_t		n = STRLEN(fname) - 4;
	Scene*		scene = GetMainScene();
	Core::String filename(fname);
	Core::String ext(filename.Right(4));
/*
 * To save the frame buffer in a bitmap, log a SaveScreen event which will
 * be handled by the scene manager thread
 */
	if (ext.CompareNoCase(TEXT(".bmp")) == 0)
	{
		LoadEvent* fe = new LoadEvent(Event::SAVE_SCREEN);
		fe->FileName = fname;
		GetMessenger()->Observe(scene, Event::SAVE_SCREEN, NULL);
		fe->Log();
		return true;
	}
	if (ext.CompareNoCase(TEXT(".vix")) == 0)
	{
		FileMessenger	savefile;

		if (savefile.Open(fname, Messenger::OPEN_WRITE))
		{
			Scene*	savescene = new Scene();
			TCHAR	filebase[VX_MaxPath];
			Model*	root = world->GetUserModels();
			Engine*	simroot = world->GetUserEngines();

			Core::Stream::ParseDirectory(fname, filebase, NULL);
			savescene->SetName(Core::String(filebase) + TEXT(".scene"));
			savescene->SetCamera(scene->GetCamera());
			savescene->SetAmbient(scene->GetAmbient());
			savescene->SetBackColor(scene->GetBackColor());
			if (root)
				root->SetName(Core::String(filebase) + TEXT(".root"));
			if (simroot)
			{
				simroot->SetName(Core::String(filebase) + TEXT(".simroot"));
				simroot->SetStartTime(0.0f, Engine::CONTROL_CHILDREN);
			}
			savescene->SetModels(root);
			savescene->SetEngines(simroot);
			return savescene->Save(savefile, 0) != 0;
		}
	}
	VX_ERROR(("MFC::Doc::SaveDocument ERROR cannot open %s for write\n", fname), 0);
}

}	// end MFC
}	// end Vixen