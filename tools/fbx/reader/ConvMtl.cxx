#include "vixen.h"
#include "ConvNode.h"
#include "FBXReader.h"

namespace Vixen { namespace FBX {

// Get specific property value and connected texture if any.
// Value = Property value * Factor property value (if no factor property, multiply by 1).
Vixen::Texture* ConvAppear::GetTextureProperty(const char* pPropertyName)
{
	Vixen::Texture*	tex = NULL;
    FbxDouble3 lResult(0, 0, 0);
    const FbxProperty lProperty = m_Material->FindProperty(pPropertyName);

    if (lProperty.IsValid())
    {
        const int lTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();
        if (lTextureCount)
        {
            const FbxFileTexture* lTexture = lProperty.GetSrcObject<FbxFileTexture>();
            if (lTexture && lTexture->GetUserDataPtr())
                return static_cast<Vixen::Texture*>(lTexture->GetUserDataPtr());
        }
    }
    return NULL;
}

// Get specific property value and connected texture if any.
// Value = Property value * Factor property value (if no factor property, multiply by 1).
FbxDouble3 ConvAppear::GetMaterialProperty(const char* pPropertyName, const char* pFactorPropertyName)
{
    FbxDouble3 lResult(0, 0, 0);
    const FbxProperty lProperty = m_Material->FindProperty(pPropertyName);
    const FbxProperty lFactorProperty = m_Material->FindProperty(pFactorPropertyName);
    if (lProperty.IsValid() && lFactorProperty.IsValid())
    {
        lResult = lProperty.Get<FbxDouble3>();
        double lFactor = lFactorProperty.Get<FbxDouble>();
        if (lFactor != 1)
        {
            lResult[0] *= lFactor;
            lResult[1] *= lFactor;
            lResult[2] *= lFactor;
        }
    }
    return lResult;
}

// Bake material properties.
Vixen::SharedObj* ConvAppear::Convert()
{
	if (!ConvNode::Convert())
		return NULL;

	Vixen::Appearance*		app = (Vixen::Appearance*) m_VixNode;
	Vixen::PhongMaterial*	mtl = (Vixen::PhongMaterial*) app->GetMaterial();
    FbxDouble3				lAmbient = GetMaterialProperty(FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor);
    FbxDouble3				lDiffuse = GetMaterialProperty(FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor);
    FbxDouble3				lSpecular = GetMaterialProperty(FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor);
    FbxDouble3				lEmissive = GetMaterialProperty(FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor);
    FbxProperty				lShininessProperty = m_Material->FindProperty(FbxSurfaceMaterial::sShininess);

	mtl->SetAmbient(Vixen::Col4(static_cast<float>(lAmbient[0]), static_cast<float>(lAmbient[1]), static_cast<float>(lAmbient[2])));
	mtl->SetDiffuse(Vixen::Col4(static_cast<float>(lDiffuse[0]), static_cast<float>(lDiffuse[1]), static_cast<float>(lDiffuse[2])));
	mtl->SetSpecular(Vixen::Col4(static_cast<float>(lSpecular[0]), static_cast<float>(lSpecular[1]), static_cast<float>(lSpecular[2])));
	mtl->SetEmission(Vixen::Col4(static_cast<float>(lEmissive[0]), static_cast<float>(lEmissive[1]), static_cast<float>(lEmissive[2])));
    if (lShininessProperty.IsValid())
    {
        double lShininess = lShininessProperty.Get<FbxDouble>();
        mtl->SetShine(static_cast<float>(lShininess));
    }
    return app;
}

// Bake material properties.
Vixen::Appearance* ConvAppear::MakeAppearance()
{
	Vixen::PhongMaterial*	mtl = new Vixen::PhongMaterial();
	Vixen::Texture*			tex = NULL;
	Vixen::Appearance*		app = new Vixen::Appearance();
	Vixen::Sampler*			smp;
    FbxDouble3				lAmbient;
    FbxDouble3				lDiffuse;
    FbxDouble3				lSpecular;
    FbxDouble3				lEmissive;

	tex = GetTextureProperty(FbxSurfaceMaterial::sDiffuse);
	if (tex != NULL)
	{
		smp = new Vixen::Sampler(tex);
		smp->Set(Vixen::Sampler::TEXTUREOP, Vixen::Sampler::DIFFUSE);
		app->Append(smp);
		VX_TRACE(FBXReader::Debug, ("Making Sampler DIFFUSE %s\n", tex->GetFileName()));
	}
	tex = GetTextureProperty(FbxSurfaceMaterial::sSpecular);
	if (tex != NULL)
	{
		smp = new Vixen::Sampler(tex);
		smp->Set(Vixen::Sampler::TEXTUREOP, Vixen::Sampler::SPECULAR);
		VX_TRACE(FBXReader::Debug, ("Making Sampler SPECULAR %s\n", tex->GetFileName()));
		app->Append(smp);
	}
	tex = GetTextureProperty(FbxSurfaceMaterial::sEmissive);
	if (tex != NULL)
	{
		smp = new Vixen::Sampler(tex);
		smp->Set(Vixen::Sampler::TEXTUREOP, Vixen::Sampler::EMISSION);
		VX_TRACE(FBXReader::Debug, ("Making Sampler EMMISSION %s\n", tex->GetFileName()));
		app->Append(smp);
	}
	app->SetMaterial(mtl);
	m_VixNode = app;
    return app;
}

Vixen::Texture* ConvTexture::MakeTexture()
{
	Vixen::Texture* tex;

	if (m_Texture == NULL)
		return NULL;
	if (m_Texture->GetFileName() == NULL)
		return NULL;
	tex = new Vixen::Texture();
	m_VixNode = tex;
	return tex;
}

Vixen::SharedObj* ConvTexture::Convert()
{
	const char*	fname;

	if (!ConvNode::Convert())
		return NULL;
	fname = m_Texture->GetFileName();
	Vixen::Core::String	fpath(fname);
	Vixen::Texture*		tex = (Vixen::Texture*) m_VixNode;
				
	if (Vixen::Core::Stream::IsRelativePath(fname))
	{
		fpath = Vixen::World::Get()->GetMediaDir();
		fpath += TEXT('/');
		fpath += fname;
	}
	tex->Load(fpath);
	return m_VixNode;
}

} }	// end FBX