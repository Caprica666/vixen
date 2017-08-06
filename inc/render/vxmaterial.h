/*!
 * @file vxmaterial.h
 * @brief Encapsulates material properties for 3D lighting.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxappear.h vxlight.h
 */
#pragma once

namespace Vixen {

/*!
 * @class Material
 * @brief Contains constant properties describing how an object responds to light.
 *
 * A material is attached to the Appearance associated with each Model in
 * the scene hierarchy. If the appearance does not enable lighting,
 * the material is ignored.
 *
 * @ingroup vixen
 * @see Appearance Light
 */
class Material : public DeviceBuffer
{
public:
	VX_DECLARE_CLASS(Material);

	Material(const TCHAR* desc = NULL, int nbytes = 0);

	virtual bool		Copy(const SharedObj* src_obj);
	virtual int			Save(Messenger&, int) const;
	virtual bool		Do(Messenger&, int);
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	enum Opcode
	{
		MAT_SetDesc = SharedObj::OBJ_NextOp,
		MAT_SetData,
		MAT_NextOp = SharedObj::OBJ_NextOp + 10,
	};
};

/*!
 * @class PhongMaterial
 * @brief Phong lighting model material properties.
 *
 * @ingroup vixen
 * @see Appearance Light Material
 */
class PhongMaterial : public Material
{
public:
	VX_DECLARE_CLASS(PhongMaterial);

//	Initializers
	PhongMaterial(const Col4* c = NULL);		//!< Construct material with given color.
	PhongMaterial(const Col4& c);				//!< Construct material with given color.
	PhongMaterial(const PhongMaterial& src);	//!< Construct material from another material.
	~PhongMaterial();

	bool operator==(const PhongMaterial&);		//!< Test materials for equality.
	bool operator!=(const PhongMaterial& src)	//!< Test materials for inequality.
		{ return !operator==(src); }

//	Material Attribute Accessors
	void	SetDiffuse(const Col4&);	//!< Set diffuse color component.
	void	SetSpecular(const Col4&);	//!< Set specular color component.
	void	SetAmbient(const Col4&);	//!< Set ambient color component.
	void	SetEmission(const Col4&);	//!< Set emissive color component.
	void	SetShine(float);			//!< Set shininess (specular power).

	float			GetShine() const	//!< Return shininess (specular power).
		{ return m_ShaderData.Shine; }
	const Col4&		GetDiffuse() const	//!< Return diffuse color.
		{ return m_ShaderData.Diffuse; }
	const Col4&		GetSpecular() const	//!< Return specular color.
		{ return m_ShaderData.Specular; }
	const Col4&		GetAmbient() const	//!< Return ambient color.
		{ return m_ShaderData.Ambient; }
	const Col4&		GetEmission() const	//!< Return emissive color.
		{ return m_ShaderData.Emission; }

//  Overrides
	virtual bool	Copy(const SharedObj* src);
	virtual bool	Do(Messenger&, int);
	virtual int		Save(Messenger&, int) const;
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*
	 * Material::Do opcodes (also for binary file format)
	 */
	enum Opcode
	{
		MAT_SetDiffuse = SharedObj::OBJ_NextOp,
		MAT_SetSpecular,
		MAT_SetAmbient,
		MAT_SetEmission,
		MAT_SetShine,
		MAT_NextOp = SharedObj::OBJ_NextOp + 20,
	};


// same as D3DMATERIAL9 structure
//	D3DCOLORVALUE   Diffuse;
//	D3DCOLORVALUE   Ambient;
//	D3DCOLORVALUE   Specular;
//	D3DCOLORVALUE   Emissive;
//	float           Power;
	struct ShaderConstants
	{
		Col4	Diffuse;
		Col4	Ambient;
		Col4	Specular;
		Col4	Emission;
		float	Shine;
		int32	HasDiffuseMap;
		int32	HasSpecularMap;
		int32	HasNormalMap;
	};

protected:
	ShaderConstants	m_ShaderData;
};

} // end Vixen