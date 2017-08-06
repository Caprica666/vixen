#pragma once

namespace Vixen {

/*!
 * @class ImageSwitch
 * @brief Texture animator that keeps an array of images and switches between them at run-time.
 *
 * You can use it wherever you can use a normal image. The default behavior is for 
 * the image switcher to automatically increment the index each frame, causing a different image
 * to be displayed every frame. You can also animate the image index with an external engine.
 *
 * @see Texture TextureSwitcher BillBoard
 */
class ImageSwitch : public Texture
{
public:
	VX_DECLARE_CLASS(ImageSwitch);

	ImageSwitch();

	Texture*		GetTexture(int);				//!< Get image based on index
	const Texture*	GetTexture(int) const;
	void			SetIndex(int);				//!< Set index of image to display
	int				GetIndex() const;			//!< Get index of image being displayed
	bool			SetTexture(int, Texture*);	//!< Set image at given index	
	int				AddImage(Texture*);			//!< Add another image at the end
	int				GetSize() const;			//!< Return number of images
	void			Empty();					//!< Remove all images

	bool			Copy(const SharedObj*);
	bool			Do(Messenger& s, int op);
	int				Save(Messenger&, int) const;
	DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	enum Opcode
	{
		IMAGESW_SetIndex = Texture::IMAGE_NextOp,
		IMAGESW_SetAt,
		IMAGESW_NextOp = Texture::IMAGE_NextOp + 10,
	};


protected:
	int32				m_Index;
	int32				m_CurImage;
	RefArray<Texture>	m_Images;
};

inline ImageSwitch::ImageSwitch() : Texture()
{
	m_CurImage = 0;
	m_Index = -1;
}

inline int ImageSwitch::GetIndex() const
{
	return m_CurImage;
}

inline int ImageSwitch::GetSize() const
{
	return m_Images.GetSize();
}

inline Texture* ImageSwitch::GetTexture(int i)
{
	return m_Images[i];
}

inline const Texture* ImageSwitch::GetTexture(int i) const
{
	return m_Images[i];
}

} // end Vixen