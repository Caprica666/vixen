/*!
 * @file vxmultitex.h
 * @brief Multi-textured appearance
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxappear.h vxappattrs.h
 */

#pragma once


#define	APPEAR_MaxApps	6
#define	APPEAR_InitLightMap		-1	// light map
#define	APPEAR_InitBumpMap		-2	// bump map
#define	APPEAR_InitReflection	-4	// reflection map
#define APPEAR_InitHilight		-8	// specular hilight

class VXAppearArray : public VXArray<VXAppearance>
{
public:
	VXAppearArray() : VXArray<VXAppearance>() { }
	~VXAppearArray() { Empty(); }

protected:
	void ConstructElems(int, int);
	void DestructElems(int, int);
	void CopyElem(int i, const void* p);
};

/*!
 * @class VXMultiTex
 *
 * Encapsulates a set of appearances to permit multiple textures to
 * be applied to a single object. Each texture is represented by a different
 * sub-appearance which can specify the image, texture coordinate
 * generation method, destination blending function, etc.
 * Textures are applied in the order the sub-appearances are added.
 * If the underlying hardware supports multiple textures simultaneously,
 * it is used effectively. If not, the geometry is rendered multiple
 * times and alpha blended for each texture effect.
 *
 * For all subappearances, attributes that are not set are
 * given default values. Attributes set in one subappearance are not
 * inherited by the others. Attributes which do not apply to texturing
 * should not be used except by the first sub-appearance because they
 * will give unpredictible and device-dependent results. The valid
 * attributes for multi-texturing are:
 * @code
 *	APPEAR_Texturing	enable/disable texturing
 *	APPEAR_MinFilter	minification filter
 *	APPEAR_MagFilter	magnification filter
 *	APPEAR_TexGen		texture coordinate generation method
 *	APPEAR_TexCoord		texture coordinate set to use
 *	APPEAR_TextureOp	texture combination operation
 * @endcode
 *
 * <B>Texture Combination</B>
 *
 * The value of  APPEAR_TextureOp determines how a texture is combined
 * with the results of the previous texturing operations. For the
 * first subappearance, it controls how the texture combines with
 * the material.
 * @code
 *	APPEAR_Replace		replace by current texture
 *	APPEAR_Modulate		multiply previous pixels by texture
 *	APPEAR_Add			add previous pixels to texture
 *	APPEAR_Blend		blend with previous pixels based on color
 * @endcode
 *
 * If you request more simultaneous texture units than the display
 * device supports in hardware, the system will render the geometry in
 * several passes. In this case, alpha blending is used to combine
 * the textures between passes. For this reason, the alpha blending
 * and transparency attributes APPEAR_Transparency, APPEAR_SrcBlend,
 * APPEAR_DstBlend should only be used by the first subappearance.
 * Materials on subappearances other than the first are only used
 * to set diffuse color - other attributes are ignored.
 *
 * @see VXAppearance VXImage
 */
class VXMultiTex : public VXAppearance
{
public:
	VXMultiTex();
	VX_DECLARE_CLASS(VXMultiTex);

//! Select a subappearance for subsequent update operations.
	bool		Select(int index);
//! Initialize multi-textured appearance
	bool		InitAt(int index, int texop, VXImage* texture);

//	VXWorldearBase Overrides
	virtual	void		Set(int attr, int val);
	virtual	bool		SetAt(int index, const VXAppearance*);
	virtual	void		SetImage(const VXImage*);
	virtual	void		SetMaterial(const VXMaterial*);
	virtual	int			GetSize() const	{ return 1 + m_Apps.GetSize(); }
	virtual	const VXAppearance*	GetAt(int index) const;
	virtual	VXAppearance*	GetAt(int index);

//	VXObj Overrides
	virtual	bool		Copy(const VXObj*);
	virtual	int			Save(VXMessenger&, int) const;
	virtual	VDebug&		Print(VDebug& = vixen_debug, int opts = VXObj::PRINT_Default) const;
	virtual	bool		Do(VXMessenger&, int);
	virtual	void		LoadDev(VXScene* scene, const VXGeometry* geo) const;

	/*
	 * MultiTex::Do opcodes
	 */
	enum Opcode
	{
		APPEAR_Select = APPEAR_LastOp,
		APPEAR_NextOp = APPEAR_LastOp + 20,
	};

protected:
	int32			m_CurApp;		// selected subappearance
	VXAppearArray	m_Apps;			// extra appearances
};

