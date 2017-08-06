/*!
 * @file vxtexswitcher.h
 * @brief Loads and controls animated textures.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vximageswitch.h vxswitcher.h
 */
#pragma once

namespace Vixen {

/*!
 * @class TextureSwitcher
 * @brief Engine designed for implementing texture animation.
 *
 * TextureSwitcher creates an ImageSwitch contaning a set of
 * bitmaps described in a configuration file. Each line in
 * the ASCII config file has the name of a bitmap file followed
 * by the number of seconds to display that bitmap.
 *
 * The ImageSwitch is both constructed and animated by the
 * TextureSwitcher. To use it, you need to attach it to an appearance
 * used by a Sprite, BillBoard or any other shape you want to texture
 * with this animated sequence.
 *
 * You can specify the name of the image file to load and the
 * TextureSwitcher will automatically use this file when it is started.
 *
 * @see Switcher ModelSwitch
*/
class TextureSwitcher : public Switcher
{
public:
	VX_DECLARE_CLASS(TextureSwitcher);

	//! Construct empty texture switcher.
	TextureSwitcher(int index = 0);

	//! Load texture animation from input file.
	ImageSwitch*	Load(const TCHAR* filename, const TCHAR* rootdir);
	ImageSwitch*	Load(const TCHAR* filename, Core::Stream* stream = NULL);

	//! Load texture animation from input string.
	ImageSwitch*	Load(Core::String&);

	//! Load texture animation as numbered frames, discard images after display.
	ImageSwitch*	LoadFrames(const TCHAR* filename, int32 maxframes = 1);

	//! Return name of texture animation description file.
	const TCHAR*	GetFileName() const
		{ return m_FileName.IsEmpty() ? NULL : (const TCHAR*) m_FileName; }

	//! Set name of texture animation description file.
	void		SetFileName(const TCHAR* filename);

	//! Load image from file and add at the end of this texture animation.
	bool		AddImage(const TCHAR* filename, float time);

	void		OnIndexChange(int newindex);
	void		UnloadAll();

	virtual bool	Do(Messenger& s, int op);
	virtual bool	Copy(const SharedObj*);
	virtual int		Save(Messenger&, int) const;
	virtual bool	OnStart();
	virtual bool	OnEvent(Event*);
	virtual bool	OnReset();
	virtual	DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*
	 * Switcher::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		TEXSW_Load = SWITCHER_NextOp,
		TEXSW_SetFileName,
		TEXSW_AddImage,
		TEXSW_LoadString,
		TEXSW_LoadFrames,
		TEXSW_NextOp = SWITCHER_NextOp + 10,
	};

protected:	
	bool		LoadNext(int32 maxframes);

	Core::String	m_FileName;
	int32			m_MaxFrames;
	int32			m_LastFrame;
	int32			m_NumLoaded;
};

} // end Vixen