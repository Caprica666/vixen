#pragma once



/*!
 * @class Material
 * @brief Encapsulates the lighting properties of a geometric object.
 *
 * The material controls how the object will respond to reflected light.
 * It is attached to the Appearance associated with each Model in
 * the scene hierarchy. If the appearance does not enable lighting,
 * the material is ignored.
 *
 * @see Appearance
 */
class Material : public PSObj
{
public:
	VX_DECLARE_CLASS(Material);

//	Initializers
	Material(const PSCol4* c = NULL);
	Material(const PSCol4& c);
	Material(const Material& src)	{ Copy(&src); }

	bool operator==(const Material&);
	bool operator!=(const Material& src) { return !operator==(src); }

//	Material Attribute Accessors
	void		SetDiffuse(const Col4&);
	void		SetSpecular(const Col4&);
	void		SetAmbient(const Col4&);
	void		SetEmission(const Col4&);
	void		SetShine(float);
	float		GetShine() const;
	Col4		GetDiffuse() const		{ return m_Diffuse; }
	Col4		GetSpecular() const		{ return m_Specular; }
	Col4		GetAmbient() const		{ return m_Ambient; }
	Col4		GetEmission() const		{ return m_Emission; }


//  Overrides
	virtual bool	Copy(const PSObj* src);
	virtual bool	Do(Messenger&, int);
	virtual int		Save(Messenger&, int) const;
	virtual DebugOut&	Print(DebugOut& = psm_debug) const;
	virtual void	LoadDev(Scene*);

	/*
	 * Material::Do opcodes (also for binary file format)
	 */
	enum Opcode
	{
		MAT_SetDiffuse = OBJ_NextOp,
		MAT_SetSpecular,
		MAT_SetAmbient,
		MAT_SetEmission,
		MAT_SetShine,
		MAT_NextOp = OBJ_NextOp + 20,
		MAT_All,
	};
protected:
	Col4		m_Diffuse;	// Diffuse color RGBA
	Col4		m_Ambient;	// Ambient color RGB
	Col4		m_Specular;	// Specular 'shininess'
	Col4		m_Emission;	// Emissive color RGB
	float	    m_Shine;	// Sharpness if specular highlight

public:	
	D3DMATERIAL9	dx_lpmat;	// DirectX material object
};

