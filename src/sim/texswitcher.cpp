#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(TextureSwitcher, Switcher, VX_TextureSwitcher);

static const TCHAR* opnames[] =
{	TEXT("Load"), TEXT("SetFileName"), TEXT("LoadString") };

const TCHAR** TextureSwitcher::DoNames = opnames;

TextureSwitcher::TextureSwitcher(int index) : Switcher()
{
	m_Index = index;
	m_MaxFrames = 0;
	m_NumLoaded = 0;
}

/*!
 * @fn ImageSwitch* TextureSwitcher::Load(const TCHAR* filename, const TCHAR* rootdir)
 * @param filename	Name of text file containing image list.
 *					If the extension of the file is IFL, the time is in frames,
 *					otherwise it is in seconds (a frame is 1/30 sec).
 * @param rootdir	Name of root directory to use for images with relative pathnames.
 *
 * Creates a ImageSwitch object that sequentially switches between the
 * texture files designated by the input text file.
 * Each line in the file has the name of a bitmap image
 * and the time (in seconds or frames) that image should be shown.
 * If the time is omitted, the time span of the switcher is used.
 * Each bitmap is converted to a Texture that is associated
 * with a ImageSwitch that allows you to select one of
 * multiple images to use for texturing.
 *
 * This function also sets the target of the texture switcher to
 * be the image switch created. The TextureSwitcher will advance
 * the image index of the ImageSwitch according to the times
 * in the input file if the engine is running. Texture loads
 * are processed independently of simulation and do not require
 * the texture switcher to be active.
 *
 * In order to see the results of the TextureSwitcher, you
 * must create a shape, put it in the scene graph
 * and make it use the ImageSwitch in its appearance.
 *
 * @return -> image switch containing bitmaps to cycle thru,
 *			NULL if bitmaps could not be opened
 *
 * @see ImageSwitch Texture Sprite Switcher::SetTimeSpan
 */
ImageSwitch* TextureSwitcher::Load(const TCHAR* filename, const TCHAR* rootdir)
{
	VX_STREAM_BEGIN(s)
      *s << OP(VX_TextureSwitcher, TEXSW_Load) << this << filename << rootdir;
    VX_STREAM_END(  )

	TCHAR			fname[VX_MaxPath];
	ImageSwitch*	imageset = (ImageSwitch*) GetTarget();

	if (rootdir &&
		(*filename != TEXT('/')) &&
		(*filename != TEXT('\\')) &&
		!STRCHR(filename, TEXT(':')))
	{
		STRCPY(fname, rootdir);
		STRCAT(fname, TEXT("/"));
		STRCAT(fname, filename);
	}
	else
		STRCPY(fname, filename);
	SetFileName(fname);
	if (imageset == NULL)
	{
		imageset = new ImageSwitch;
		SetTarget(imageset);
	}
	imageset->SetFileName(fname);
	if (!imageset->HasColorKey())
		imageset->SetTranspColor(Col4(0,0,0));
	GetMessenger()->Observe(this, Event::LOAD_TEXT, this);
	World3D::Get()->LoadAsync(fname, this);
	return imageset;
}

/*!
 * @fn ImageSwitch* TextureSwitcher::Load(const TCHAR* filename, Core::Stream* stream)
 * @param filename	Name of text file containing image list.
 *					If the extension of the file is IFL, the time is in frames,
 *					otherwise it is in seconds (a frame is 1/30 sec).
 * @param stream	Ignored (for compatibility with parent class function).
 *
 * Creates a ImageSwitch object that sequentially switches between the
 * texture files designated by the input text file.
 * Each line in the file has the name of a bitmap image
 * and the time (in seconds or frames) that image should be shown.
 * If the time is omitted, the time span of the switcher is used.
 * Each bitmap is converted to a Texture that is associated
 * with a ImageSwitch that allows you to select one of
 * multiple images to use for texturing.
 *
 * This function also sets the target of the texture switcher to
 * be the image switch created. The TextureSwitcher will advance
 * the image index of the ImageSwitch according to the times
 * in the input file if the engine is running. Texture loads
 * are processed independently of simulation and do not require
 * the texture switcher to be active.
 *
 * In order to see the results of the TextureSwitcher, you
 * must create a shape, put it in the scene graph
 * and make it use the ImageSwitch in its appearance.
 *
 * @return -> image switch containing bitmaps to cycle thru,
 *			NULL if bitmaps could not be opened
 *
 * @see ImageSwitch Texture Sprite Switcher::SetTimeSpan
 */
ImageSwitch* TextureSwitcher::Load(const TCHAR* filename, Core::Stream* stream)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_TextureSwitcher, TEXSW_Load) << this << filename << (TCHAR*) NULL;
	VX_STREAM_END(  )

	ImageSwitch* imageset = (ImageSwitch*) GetTarget();

	SetFileName(filename);
	if (imageset == NULL)
	   {
		imageset = new ImageSwitch;
		SetTarget(imageset);
	   }
	if (imageset->GetFileName() == NULL)
		imageset->SetFileName(filename);
	if (!imageset->HasColorKey())
		imageset->SetTranspColor(Col4(0,0,0));
	GetMessenger()->Observe(this, Event::LOAD_TEXT, this);
	World3D::Get()->LoadAsync(filename, this);
	return imageset;
}


/*!
 * @fn ImageSwitch* TextureSwitcher::Load(String& input_data)
 * @param strdata	String containing the image list.
 *
 * Creates a ImageSwitch object that sequentially switches between the
 * texture files designated by the input text file.
 * Each line in the input string has the name of a bitmap image
 * and the time (in seconds or frames) that image should be shown.
 * If the time is omitted, the time span of the switcher is used.
 * Each bitmap is converted to a Texture that is associated
 * with a ImageSwitch that allows you to select one of
 * multiple images to use for texturing.
 *
 * This function also sets the target of the texture switcher to
 * be the image switch created. The TextureSwitcher will advance
 * the image index of the ImageSwitch according to the times
 * in the input file if the engine is running. Texture loads
 * are processed independently of simulation and do not require
 * the texture switcher to be active.
 *
 * In order to see the results of the TextureSwitcher, you
 * must create a shape, put it in the scene graph
 * and make it use the ImageSwitch in its appearance.
 *
 * @return -> image switch containing bitmaps to cycle thru,
 *			NULL if bitmaps could not be opened
 *
 * @see ImageSwitch Texture Sprite Switcher::SetTimeSpan
*/
ImageSwitch* TextureSwitcher::Load(Core::String& input_data)
{
	ImageSwitch*	imageset = (ImageSwitch*) GetTarget();
  	int				imagenum = 0;			// number of images in set
	float			time = 0.0f;
	TCHAR			fname[VX_MaxString];
	TCHAR			rootdir[VX_MaxPath];
	const TCHAR*	p = input_data;
	size_t			n;

	if (imageset == NULL)
	{
		imageset = new ImageSwitch;
		imageset->SetFileName(m_FileName);
		SetTarget(imageset);
	}
	
	if (!imageset->HasColorKey())
		imageset->SetTranspColor(Col4(0,0,0));
	if (input_data.IsEmpty())
		return NULL;
	Core::Stream::ParseDirectory(m_FileName, NULL, rootdir);
	if (*rootdir == 0)
		STRCPY(rootdir, World::Get()->GetLoader()->GetDirectory());
	while (*p)							// get next line in image set file
	{
		while (isspace(*p))				// skip blank lines
			if (*++p == 0)
				break;
		if (*p == 0)
			continue;
		if ((p[0] == TEXT('/')) && (p[1] == TEXT('/')))
			continue;					// skip comments, too
		n = STRCSPN(p, TEXT(" \t\r\n;"));		// get length of filename
		if (rootdir &&
			(*p != TEXT('/')) &&
			(*p != TEXT('\\')) &&
			!STRCHR(p, TEXT(':')))
		{
			STRCPY(fname, rootdir);
			STRCAT(fname, TEXT("/"));
			STRNCAT(fname, p, n);
		}
		else
		{
			STRNCPY(fname, p, n);		// save filename
			fname[n] = 0;
		}
		p += n;							// -> after filename	
		time = (float) ATOF(p);			// parse time to show image
		if (time == 0.0f)				// no time?
			time = 1.0f;				// default is 1
		if (AddImage(fname, time))		// add the image	
			++imagenum;					// count it
		p += STRCSPN(p, TEXT("\r\n;"));	// skip to next line
	}
	if (imagenum > 0)					// have some images in set?
		return imageset;
	return NULL;
}

bool TextureSwitcher::AddImage(const TCHAR* fname, float time)
{
	VX_STREAM_BEGIN(s)
      *s << OP(VX_TextureSwitcher, TEXSW_AddImage) << this << fname << time;
    VX_STREAM_END()

	ImageSwitch* imageset = (ImageSwitch*) GetTarget();

	if (imageset == NULL)
		return false;
	Texture* image = new Texture;
	imageset->AddImage(image);			// add image to image set
	if (GetTimeSpan() >= 0.0f)
	   {
		if (GetTimeSpan() > 0.0f)
			time *= GetTimeSpan();
		Add(time);						// add time to the switcher
	   }
	image->Load(fname);
	return true;
}

ImageSwitch* TextureSwitcher::LoadFrames(const TCHAR* filename, int32 maxframes)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_TextureSwitcher, TEXSW_LoadFrames) << this << filename << maxframes;
	VX_STREAM_END(  )

	ImageSwitch* imageset = (ImageSwitch*) GetTarget();
	SetFileName(filename);
	m_MaxFrames = maxframes;
	if (maxframes < 2)
		maxframes = 2;
	m_LastFrame = 0;
	if (imageset == NULL)
	{
		imageset = new ImageSwitch;
		SetTarget(imageset);
	}
	imageset->MakeLock();
	GetMessenger()->Observe(this, Event::LOAD_TEXT, NULL);
	LoadNext(maxframes);
	return imageset;
}

bool TextureSwitcher::LoadNext(int32 maxframes)
{
	ImageSwitch* imageset = (ImageSwitch*) GetTarget();
	TCHAR			namebuf[VX_MaxPath];
	int32			curframe;
	size_t			n;
	const TCHAR*	fname = GetFileName();

	if ((imageset == NULL) || !imageset->IsClass(VX_ImageSwitch) || (fname == NULL))
		return false;
	for (int i = 0; i < maxframes; ++i)
	{
		Texture* image = new Texture;
		curframe = imageset->AddImage(image) + 1;
		STRCPY(namebuf, fname);
		n = STRLEN(fname) - 4;
		TCHAR* p = STRCHR(namebuf, TEXT('$'));
		if (p)
		{
			n = p - namebuf;
			SPRINTF(p, TEXT("%04d%s"), ++m_LastFrame, fname + n + 1);
		}	
		else
			SPRINTF(namebuf + n, TEXT("%d%s"), ++m_LastFrame, fname + n);	
		World3D::Get()->LoadAsync(namebuf, image);
	}
	return true;
}

void TextureSwitcher::UnloadAll()
{
	ImageSwitch* imageset = (ImageSwitch*) GetTarget();
	int32			n = 0;
	const TCHAR*	fname = GetFileName();

	if ((imageset == NULL) || !imageset->IsClass(VX_ImageSwitch) || (fname == NULL))
		return;
	imageset->Empty();
	m_LastFrame = 0;
}

bool TextureSwitcher::OnEvent(Event* event)
{
	ImageSwitch* imageset = (ImageSwitch*) GetTarget();
	LoadEvent*	e = (LoadEvent*) event;
	const SharedObj*	sender = e->Sender;

	switch (event->Code)
	{
		case Event::LOAD_IMAGE:
		++m_NumLoaded;
		return true;

		case Event::LOAD_TEXT:
		if (sender != this)
			return false;
		if (imageset == NULL)
			return true;
		VX_ASSERT(imageset->IsClass(VX_ImageSwitch));
		VX_TRACE(FileLoader::Debug, ("TextureSwitcher::Load %s complete\n", LPCTSTR(e->FileName)));
		GetMessenger()->Ignore(this, Event::LOAD_TEXT, this);
		SetFileName(e->FileName);
		Load(e->Text);
		return true;
	}
	return false;
}

void TextureSwitcher::OnIndexChange(int index)
{
	ImageSwitch* imageset = (ImageSwitch*) GetTarget();
	const Texture*	image;
	void*	bitmap;
	int		type;
	int		n;

	if ((imageset == NULL) || !imageset->IsClass(VX_ImageSwitch))
		return;
	if (m_MaxFrames <= 0)				// doing frame buffering?
	{
		Switcher::OnIndexChange(index);
		return;
	}
	if (index >= m_NumLoaded)			// not loaded yet?
	{
		if (index - 1 <= m_NumLoaded)
			return;
		index = m_NumLoaded;			// just display last one loaded
	}
	ObjectLock lock(imageset);
	image = imageset->GetTexture(index);	// switch to this image if it is valid	
	if (image)
	{
		bitmap = image->GetBitmap(&type);	
		if (bitmap == NULL)
		{
			if (type == Bitmap::ISLOADING)
				return;
			if (type == Bitmap::NOTEXTURE)
				return;
		}
		LoadNext(1);					// load another frame
	}
	else
	{
		LoadNext(1);
		return;
	}
	n = index - 2;
	if (n > 0)							// delete images already used
	{
		Texture* image = imageset->GetTexture(n);
		if (image)
		{
			imageset->SetTexture(n, NULL);
			World3D::Get()->GetLoader()->Unload(image->GetFileName());
		}
	}
	Switcher::OnIndexChange(index);
}

bool TextureSwitcher::OnStart()
{
	ImageSwitch* target = (ImageSwitch*) GetTarget();

	const TCHAR* filename = GetFileName();
	if (filename && (target == NULL))
		Load(filename, (const TCHAR*) NULL);
	Switcher::OnStart();
	if (m_MaxFrames > 0)
		SetDuration(0);
	return true;
}

bool TextureSwitcher::OnReset()
{
	if (m_MaxFrames > 0)
		UnloadAll();
	return Switcher::OnReset();
}

/*!
 * @fn void TextureSwitcher::SetFileName(const TCHAR* filename)
 * @param filename	filename of image description file
 *
 * Sets the name of the file describing the images and
 * times to display them. If the filename is set when this
 * engine starts, it will automatically load the images and
 * start displaying them.
 *
 * @see TextureSwitcher::Load
 */
void TextureSwitcher::SetFileName(const TCHAR* filename)
{
	VX_STREAM_BEGIN(s)
      *s << OP(VX_TextureSwitcher, TEXSW_SetFileName) << this << filename;
    VX_STREAM_END(  )

	if (filename == NULL)
		m_FileName.Empty();
	else
		m_FileName = filename;
}

/****
 *
 * class TextureSwitcher override for SharedObj::Copy
 *
 ****/
bool TextureSwitcher::Copy(const SharedObj* srcobj)
{
	ObjectLock dlock(this);
	ObjectLock slock(srcobj);
	if (!Switcher::Copy(srcobj))
		return false;
	const TextureSwitcher* src = (const TextureSwitcher*) srcobj;
	SetFileName(src->GetFileName());
	return true;
}

/****
 *
 * class TextureSwitcher override for SharedObj::Save
 *
 ****/
int TextureSwitcher::Save(Messenger& s, int opts) const
{
	int32 h = Switcher::Save(s, opts);
	if (h <= 0)
		return h;
	const TCHAR* filename = GetFileName();
	uint16	opcode = TEXSW_Load;

	if (opts)
		opcode = TEXSW_SetFileName;
	if (filename)
 		s << OP(VX_TextureSwitcher, opcode) << h << filename << (TCHAR*) NULL;
	return h;
}

DebugOut& TextureSwitcher::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Engine::Print(dbg, opts & ~PRINT_Trailer);
	const TCHAR* filename = GetFileName();

	if (filename)
		endl(dbg << "\t<attr name='FileName'>" << filename << "</attr>");
	Engine::Print(dbg, opts & PRINT_Trailer);
	return dbg;
}

/****
 *
 * class TextureSwitcher override for SharedObj::Do
 *
 ****/
bool TextureSwitcher::Do(Messenger& s, int op)
{
	TCHAR			str[VX_MaxString];
	TCHAR			rootdir[VX_MaxPath];
	float			time;
	Core::String	texscript;
	int32			n;

	switch (op)
	{
		case TEXSW_Load:
		s >> str >> rootdir;
		if (*rootdir)
			Load(str, rootdir);
		else
			Load(str);
		break;

		case TEXSW_LoadString:
		s >> str;
		if (str == NULL)
			break;
		texscript = str;
		Load(texscript);
		break;	

		case TEXSW_SetFileName:
		s >> str;
		SetFileName(str);
		break;

		case TEXSW_AddImage:
		s >> str >> time;
		AddImage(str, time);
		break;

		case TEXSW_LoadFrames:
		s >> str >> n;
		LoadFrames(str, n);
		break;

		default:
		return Switcher::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
			<< TextureSwitcher::DoNames[op - TEXSW_Load]	<< " " << this);
#endif
	return true;
}
}	// end Vixen