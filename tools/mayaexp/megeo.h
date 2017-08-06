#pragma once

#include <maya/MBoundingBox.h>

#include "menode.h"
class meConvState;
class meConvUV;

/*!
 * @class meConvGeo
 * @brief Base converter for all geometry (meshes and surfaces).
 *
 * This class encapsulates common code to manager shaders and UV mapping.
 * Subclasses must implement geometry conversion.
 *
 * @see meConvMesh meConvPoly meConvSurf
 */
class meConvGeo : public meConvNode
{
public:
    struct ShaderInfo
    {
		ShaderInfo();

		int				Offset;				// current vertex offset for geometry
		int				NumVerts;			// current number of unique vertices for geometry
		int				Size;				// index table size of GeoSet
		int				NumUVSets;			// number of texcoord sets in GeoSet
		int				ShaderIndex;		// index of this shader (0 - m_NumShaders)
		Ref<Shape>		Shape;				// -> Vixen shape that owns the geometry
		Ref<Geometry>	Geom;				// -> Vixen geometry
		MObject			MayaShader;			// Maya shader object
		meConvState*	ShaderConv;					// converter for shader
		meConvUV*		UVMappers[ME_MAX_TEXTURES]; // UV mapper for each texcoord set
		int				UVSets[ME_MAX_TEXTURES];    // Maya UV set to use as input
    };

    meConvGeo(const MDagPath& mpath);

protected:
    virtual bool	FindShaders() { return false; };
    virtual void	LinkUVMappers(ShaderInfo* sinfo, int nuvs);
    Appearance*		MakeAppearance(ShaderInfo* sinfo);

    Vec3			m_Center;		// center of object
    MSpace::Space	m_Space;		// space for vertex/normal info
};

