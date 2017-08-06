/****
 *
 * Utilities for Graphical Object Generation
 *
 ****/
#include "vixen.h"

namespace Vixen {


static Vec3 c[8] =
{
    Vec3(-1.0, -1.0, 1.0),          /* 0 */
    Vec3(-1.0, 1.0, 1.0),           /* 1 */
    Vec3(1.0, 1.0, 1.0),            /* 2 */
    Vec3(1.0, -1.0, 1.0),           /* 3 */
    Vec3(-1.0, -1.0, -1.0),         /* 4 */
    Vec3(-1.0, 1.0, -1.0),          /* 5 */
    Vec3(1.0, 1.0, -1.0),           /* 6 */
    Vec3(1.0, -1.0, -1.0),          /* 7 */
};

/*!
 * @fn bool GeoUtil::Rect(float w, float h, float xtex, float ytex)
 * @param mesh		triangle mesh to add the block to
 * @param width		width of rectangle
 * @param height	height of rectangle
 * @param xtex		maximum texture coordinate in X direction, defaults to 1
 * @param ytex		maximum texture coordinate in Y direction, defaults to 1
 *
 * Creates a 2D rectangle of the given dimensions and store the
 * vertices and indices in the input triangle mesh.
 * The upper left corner of the rectangle is -width / 2, height / 2
 * and the lower left is width / 2, -height / 2
 *
 * @return true if block successfully added, else false
 *
 * @see GeoUtil::Block
 */
bool GeoUtil::Rect(TriMesh* mesh, float w, float h, float xtex, float ytex)
{
	Vec3		vtx[4];		// locations
	Vec2		tc[4];		// texture coordinates
	int32		inds[6] = { 0, 2, 1, 0, 3, 2 };
	int			vecsize = GetMessenger()->SysVecSize;

/*
 * Initialize vertices based on width and height.
 * Lower left corner is at -w, -h with texcoord 1,0
 * Upper left corner is at -w, h with texcoord 0,0
 */
	w /= 2; h /= 2;
	vtx[0].Set(-w, h, 0);
	vtx[1].Set(w, h, 0);
	vtx[2].Set(w, -h, 0);
	vtx[3].Set(-w, -h, 0);
	tc[0].Set(0, ytex);
	tc[1].Set(xtex, ytex);
	tc[2].Set(xtex, 0);
	tc[3].Set(0, 0);
	mesh->AddVertices(NULL, 4);
/*
 * Copy the vertices for each primitive and add the primitive
 */
	VertexPool::Iter iter(mesh->GetVertices());
	int style = mesh->GetStyle();
	for (int j = 0; j < 4; j++)
	{
		float* vptr = iter.GetVtx(j);

		*((Vec3*) vptr) = vtx[j];
		if (vecsize == 4)
			vptr[3] = 1.0f;
		vptr += vecsize;
	    if (style & VertexPool::NORMALS)
		{
			*((Vec3*) vptr) = Vec3(0, 0, 1);
			if (vecsize == 4)
				vptr[3] = 1.0f;
			vptr += vecsize;
		}
		if (style & VertexPool::TEXCOORDS)
		{
			*vptr++ = tc[j].x;
			*vptr++ = tc[j].y;
		}
	}
	mesh->AddIndices(inds, 6);
	return true;
}

/*!
 * @fn bool GeoUtil::Block(const Vec3& size)
 * @param mesh	triangle mesh to add the block to
 * @param size	dimensions of block in x, y and z
 *				the block will be -size to +size in each dimension
 *
 * Creates a 3D block of the given dimensions and store the
 * vertices and indices in the input triangle mesh.
 *
 * @return true if block successfully added, else false
 *
 * @see GeoUtil::Ellipsoid GeoUtil::Cylinder GeoUtil::Torus
 */
bool GeoUtil::Block(TriMesh* mesh, const Vec3& size)
{
	static Vec3 n[6] = 
	{
		Vec3( 0.0, 0.0, -1.0 ),
		Vec3( 1.0, 0.0, 0.0 ),
		Vec3( 0.0, 0.0, 1.0 ),
		Vec3( -1.0, 0.0, 0.0 ),
		Vec3( 0.0, 1.0, 0.0 ),
		Vec3( 0.0, -1.0, 0.0 )
	};

	static uint16 v_list[6][6] = {
		4, 5, 6, 4, 6, 7,           /* back */
		7, 6, 2, 3, 7, 2,           /* right */
		3, 2, 1, 3, 1, 0,           /* front */
		0, 1, 5, 0, 5, 4,           /* left */
		2, 6, 5, 2, 5, 1,           /* top */
		7, 3, 0, 7, 0, 4,           /* bottom */
	};

	static Vec2 t[4] =
	{
	   Vec2(0.0, 1.0),
	   Vec2(0.0, 0.0),
	   Vec2(1.0, 0.0),
	   Vec2(1.0, 1.0)
	};

	static uint16 t_list[6] = { 3, 2, 1, 3, 1, 0 };

/*
 * Initialize vertices based on size
 */
	Vec3	vtx[8];			/* individual vertex values */
    int		i, j;
	int		style = mesh->GetStyle();
	int		vecsize = GetMessenger()->SysVecSize;

    for (i = 0; i < 8; i++)
	{
        vtx[i].x = size.x * c[i].x / 2.0f;
        vtx[i].y = size.y * c[i].y / 2.0f;
        vtx[i].z = size.z * c[i].z / 2.0f;
	}
	mesh->AddVertices(NULL, 36);
/*
 * Copy the vertices for each face
 */
	VertexPool::Iter iter(mesh->GetVertices());
    for (i = 0; i < 6; i++)
	{
        for (j = 0; j < 6; j++)
		{
			float* vptr = iter.Next();
			*((Vec3*) vptr) =  vtx[v_list[i][j]];
			if (vecsize == 4)
				vptr[3] = 1.0f;
			vptr += vecsize;
			if (style & VertexPool::NORMALS)
			{
				*((Vec3*) vptr) = n[i];
				if (vecsize == 4)
					vptr[3] = 0.0f;
				vptr += vecsize;
			}
			if (style & VertexPool::TEXCOORDS)
				*((Vec2*) vptr) = t[t_list[j]];
		}
	}
	return true;
}

/*!
 * @fn bool GeoUtil::QuadMesh(TriMesh* mesh, int rows, int cols, const float* vtx, int vtxidx = 0)
 * @param mesh		triangle mesh to add the quad mesh to
 * @param vtx		-> first vertex in the list, If NULL, no vertices
 *					are added but the indices are generated.
 * @param rows		number of rows in the quad mesh (X size)
 * @param cols		number of columns in the quad mesh (Y size)
 * @param vtxidx	index of first vertex used by quad mesh
 *
 * If vertex data is supplied, it is added along with the primitives
 * generated that compose the quad mesh. It is assumed the array contains
 * row * cols vertices. The  Style attribute of the vertex array determines the
 * vertex size and format. Index data is automatically
 * generated by assuming the vertices for the primitive
 * are in order.
 *
 * @return  true if quad mesh added, else  false
 *
 * @see Mesh::AddVertices Mesh::SetStyle
 */
bool GeoUtil::QuadMesh(TriMesh* mesh, int rows, int cols, const float* vtx, intptr firstvtx)
{
	VX_ASSERT(rows > 0);
	VX_ASSERT(cols > 0);
	if (vtx != NULL)
	{
		if (mesh->AddVertices(vtx, rows * cols) < 0)
			return false;						/* could not add vertices? */
	}
	for (int j = 0; j < rows - 1; ++j)			/* for each row */
	{
		for (int i = 0; i < cols - 1; ++i)		/* add 2 triangles for each cell */
		{
			intptr v = firstvtx + j * cols + i;
			mesh->AddIndex(v);
			mesh->AddIndex(v + cols + 1);
			mesh->AddIndex(v + 1);
			mesh->AddIndex(v);
			mesh->AddIndex(v + cols);
			mesh->AddIndex(v + cols + 1);
		}
	}
	return true;
}

/*!
 * @fn bool TriMesh::Torus(float irad, float orad, int res)
 * @param mesh	triangle mesh to add the torus to
 * @param irad	inner radius (of the hole)
 * @param orad	outer radius of torus
 * @param res	size of torus quad mesh (number of rows, columns)
 *
 * Creates a 3D torus of the given dimensions and store the
 * vertices and indices in the input triangle mesh.
 *
 * @return  true if torus successfully added, else  false
 *
 * @see GeoUtil::Ellipsoid GeoUtil::Cylinder GeoUtil::Block
 */
bool GeoUtil::Torus(TriMesh* mesh, float irad, float orad, int res)
{
	float	r, *ctab, *stab;
	Vec3	p, center, norm;
    int		i, j;
	intptr	vidx;
	int		vecsize = GetMessenger()->SysVecSize;

	if (res < 4) res = 4;				/* min res = 4, typical = 20 */
	ctab = new float[res];				/* cosine table */
	stab = new float[res];				/* sine table */
	if (ctab == NULL || stab == NULL)
		VX_ERROR(("GeoUtil::Torus: out of memory\n"), false);
	vidx = mesh->AddVertices(NULL, res * res);
	if (vidx < 0)
		VX_ERROR(("GeoUtil::Torus: out of memory\n"), false);
/*
 * Initialize the sine and cosine tables
 */
	--res;
	#pragma omp PARALLEL_FOR(res)
	cilk_for (int k = 0; k <= res; k++)
	{
        double angle = k * 2.0 * PI / res;
        ctab[k] = (float) cos(angle);
        stab[k] = (float) sin(angle);
	}
/*
 * Populate the quadmesh with locations, normals, and texture coordinates
 */
	VertexPool::Iter iter(mesh->GetVertices());
	int style = mesh->GetStyle();
	for (i = 0; i <= res; i++)
	    for (j = 0; j <= res; j++)
		{
			float* vptr = iter.Next();
			r = orad + irad * ctab[res - j];	/* compute location */
			p.Set( r * ctab[i], irad * stab[res - j], r * stab[i]);
			*((Vec3*) vptr) = p;
			if (vecsize == 4)
				vptr[3] = 1.0f;
			vptr += vecsize;
			if (style & VertexPool::NORMALS)			/* compute normal */
			{
				// find center of torus
				center.Set( orad * ctab[i], 0.0f, orad * stab[i]);
				norm = p - center;
				norm.Normalize();
				*((Vec3*) vptr) =  norm;
			if (vecsize == 4)
				vptr[3] = 0.0f;
				vptr += vecsize;
			}
			if (style & VertexPool::TEXCOORDS)			/* compute tex coords */
			{
				 *vptr++ = float(i) / res;
				 *vptr++ = float(j) / res;
			}
		}
	delete [] ctab;
	delete [] stab;
	GeoUtil::QuadMesh(mesh, res + 1, res + 1, NULL, vidx);
    return true;
}

/*!
 * @fn bool GeoUtil::Cylinder(int parts, float tradius, float bradius, float height, int res)
 * @param mesh		triangle mesh to add the cylinder to
 * @param parts		which parts of the cylinder to draw, one or more of:
 *					TOP, SIDES, BASE or ALLPARTS
 * @param tradius	radius of top of cylinder
 * @param bradius	radius of base of cylinder
 * @param height	height of cylinder
 * @param res		indicates how finely to tesselate (bigger means finer)
 *
 * Creates a 3D block of the given dimensions and store the
 * vertices and indices in the input triangle mesh.
 *
 * @return  true if cylinder successfully added, else  false
 *
 * @see GeoUtil::GeoUtil GeoUtil::Torus GeoUtil::Block
 */
bool GeoUtil::Cylinder(TriMesh* mesh, int parts, float tradius,
						float bradius, float height, int res)
{
	float	top_y = height / 2, bottom_y = -height / 2;
	float	*x, *z, angle;
	float	l, d, xzfrac, yfrac;
	Vec3	p;
	Vec3	v;
	int		style = mesh->GetStyle();
	int		size = 4 * (res + 1);
	intptr	firstvtx;
	int		vecsize = GetMessenger()->SysVecSize;

	if (bradius == 0.0 && tradius == 0.0)	/* if both radii are zero */
        return false;						/* its not a cone (is a line) */
	d = bradius - tradius;
	l = sqrtf(height * height + d * d);
	yfrac = d / l;
	xzfrac = height / l;
	x = new float[res + 1];
	z = new float[res + 1];
	if ((x == NULL) || (z == NULL) ||
		((firstvtx = mesh->AddVertices(NULL, size)) < 0))
		VX_ERROR(("GeoUtil::Cylinder: not enough memory\n"), false);
	#pragma omp PARALLEL_FOR(res)
	cilk_for (int i = 0; i <= res; i++)
	{
		angle = i * 2.0f * PI / res;
		x[i] = cosf(angle);
		z[i] = sinf(angle);
	}
	VertexPool::Iter iter(mesh->GetVertices());
/*
 * create sides
 */
	if ((parts & GeoUtil::CYL_SIDE) && (bradius != 0)) 
	{
		v.Set(0, -1, 0);
		for (int i = 0; i <= res; i++)
		{
			float*	vptr = iter.Next();
			p.Set(bradius * x[i], bottom_y, bradius * z[i]);
			*((Vec3*) vptr) = p;
			if (vecsize == 4)
				vptr[3] = 1.0f;
			vptr += vecsize;
			if (style & VertexPool::NORMALS)
			{
				v.Set(xzfrac * x[i], yfrac, xzfrac * z[i]);
				*((Vec3*) vptr) = v;
				if (vecsize == 4)
					vptr[3] = 1.0f;
				vptr += vecsize;
			}
			if (style & VertexPool::TEXCOORDS)
			{
				*vptr++ = float(i) / res;
				*vptr++ = 1;
			}
			p.Set(tradius * x[i], top_y, tradius * z[i]);
			vptr = iter.Next();
			*((Vec3*) vptr) = p;
			if (vecsize == 4)
				vptr[3] = 1.0f;
			vptr += vecsize;
			if (style & VertexPool::NORMALS)
			{
				*((Vec3*) vptr) = v;
				if (vecsize == 4)
					vptr[3] = 1.0f;
				vptr += vecsize;
			}
			if (style & VertexPool::TEXCOORDS)
			{
				*vptr++ = float(i) / res;
				*vptr++ = 0.0f;
			}
		}
		for (int i = 0; i <= res; i++)
		{
			intptr v = firstvtx + i * 2;
			mesh->AddIndex(v);
			mesh->AddIndex(v + 1);
			mesh->AddIndex(v + 3);
			mesh->AddIndex(v);
			mesh->AddIndex(v + 3);
			mesh->AddIndex(v + 2);
		}
	}
/*
 * create top
 */
	if ((parts & GeoUtil::CYL_TOP) && (tradius != 0.0))
	{
		firstvtx = 2 * res;
		v.Set(0, 1, 0);
		iter.Reset();
		for (int i = 0; i <= res; i++)
		{
			float* vptr = iter.Next();
			p.Set(tradius * x[i], top_y, tradius * z[i]);
			*((Vec3*) vptr) = p;
			if (vecsize == 4)
				vptr[3] = 1.0f;
			vptr += vecsize;
			if (style & VertexPool::NORMALS)
			{
				*((Vec3*) vptr) = v;
				if (vecsize == 4)
					vptr[3] = 1.0f;
				vptr += vecsize;
			}
			if (style & VertexPool::TEXCOORDS)
			{
				*vptr++ = 0.5f + x[i] / 2;
				*vptr++ = 0.5f + z[i] / 2;
			}
		}
		for (int i = 0; i < res; i++)
		{
			intptr v = firstvtx + i + 1;
			mesh->AddIndex(firstvtx);
			mesh->AddIndex(v);
			mesh->AddIndex(v + 1);
		}
	}
/*
 * create base
 */
    if ((parts & GeoUtil::CYL_BASE) && (bradius != 0.0))
	{
		firstvtx += res + 1;;
		v.Set(0, -1, 0);
		iter.Reset();
        for (int i = 0; i <= res; i++)
		{
			float* vptr = iter.Next();
			p.Set(bradius * x[i], bottom_y, bradius * z[i]);
			*((Vec3*) vptr) = p;
			if (vecsize == 4)
				vptr[3] = 1.0f;
			vptr += vecsize;
			if (style & VertexPool::NORMALS)
			{
				*((Vec3*) vptr) = v;
				if (vecsize == 4)
					vptr[3] = 1.0f;
				vptr += vecsize;
			}
			if (style & VertexPool::TEXCOORDS)
			{
				*vptr++ = 0.5f + x[i] / 2;
				*vptr++ = 0.5f + z[i] / 2;
			}
		}
		for (int i = 0; i < res; i++)
		{
			intptr v = firstvtx + i + 1;
			mesh->AddIndex(firstvtx);
			mesh->AddIndex(v);
			mesh->AddIndex(v + 1);
		}
	}
/*
 * Free memory areas used
 */
	delete [] x;
	delete [] z;
	return true;
}

/*!
 * @fn bool GeoUtil::Ellipsoid(TriMesh* mesh, const Vec3& size, int res)
 * @param mesh	mesh to add ellipsoid vertices to
 * @param size	diameters of ellipsoid in X, Y and Z
 * @param res	resolution, indicates how finely to tesselate (bigger means finer)
 *
 * The 3D ellipsoid is composed of triangle strips.
 *
 * @returns  true if ellipsoid successfully added, else  false
 *
 * @see GeoUtil::Cylinder GeoUtil::Torus GeoUtil::Block
 */
bool GeoUtil::Ellipsoid(TriMesh* mesh, const Vec3& size, int res)
{
	int32		i, j;
	float		res2, pires;
	Vec3		tmp;
	int			style = mesh->GetStyle();
	intptr		firstvtx;
	int			vecsize = GetMessenger()->SysVecSize;

	if (res < 4)
		res = 4;
	res2 = (res - 1) / 2.0f;
	pires = 2.0f * asinf(1.0f) / (res - 1);
	firstvtx = mesh->AddVertices(NULL, res * res);
	if (firstvtx < 0)
		VX_ERROR(("GeoUtil::Ellipsoid: not enough memory\n"), false);
	VertexPool::Iter iter(mesh->GetVertices());
	for (i = 0; i < res; i++)
	{
		float tmpr = cosf((i - res2) * pires);
		float tmpy = sinf((i - res2) * pires);
		for (j = 0; j < res; j++)
		{
			float	tmpx = sinf(-j * 2 * pires);	/* compute vertex location */
			Vec3	v(tmpr * tmpx, tmpy, tmpr * cosf(-j * 2 * pires));
			Vec3	p(size.x * v.x, size.y * v.y, size.z * v.z);
			float*	vptr = iter.Next();

			*((Vec3*) vptr) = p;
			if (vecsize == 4)
				vptr[3] = 1.0f;
			vptr += vecsize;
			if (style & VertexPool::NORMALS)
			{
				*((Vec3*) vptr) = v;
				if (vecsize == 4)
					vptr[3] = 1.0f;
				vptr += vecsize;
			}
			if (style & VertexPool::TEXCOORDS)		/* compute tex coords */
			{
				*vptr++ = 0.5f + float(j) / res;
				*vptr++ = 1.0f - float(i) / res;
			}
		}
    }
	QuadMesh(mesh, res, res, NULL, firstvtx);
	return true;
}


/*!
 *  Recursively subdivide a given triangle into four equal triangles based
 *  off of the midpoints of the given triangle.  Meant for approximating a 
 *  sphere.  Righthand rule applied to create outware pointing faces.  Add
 *  the triangle vertices to the current TriMesh once proper recursion "depth"
 *  has been reached
 */
void GeoUtil::subdivide(int32 idx[], Vec3 v1, Vec3 v2, Vec3 v3, 
					  int32 depth, int32 *numverts, bool hemi,
					  TriMesh* curtm, float fRadius)
{
	Vec3 v12, v23, v31;
	int32 i1[3], i2[3], i3[3], i4[3];
	int32 vnum12, vnum23, vnum31;
	VertexPool::Iter iter(curtm->GetVertices());

	// If a hemisphere was requested don't waste time on any triangle
	// whose vertices are below z = 0
	if (hemi && (v1.z <= 0) && (v2.z <= 0) && (v3.z <= 0)) return;

	if (depth > 0)
	{
		vnum12 = (*numverts)++; 
		vnum23 = (*numverts)++; 
		vnum31 = (*numverts)++;

		v12 = v1 + v2;
		v23 = v2 + v3;
		v31 = v3 + v1;

		i1[0] = idx[0]; i1[1] = vnum12; i1[2] = vnum31;
		i2[0] = idx[1]; i2[1] = vnum23; i2[2] = vnum12;
		i3[0] = idx[2]; i3[1] = vnum31; i3[2] = vnum23;
		i4[0] = vnum12; i4[1] = vnum23; i4[2] = vnum31;

		v12.Normalize(); v23.Normalize(); v31.Normalize();
		subdivide(i1, v1, v12, v31, depth-1, numverts, hemi, curtm, fRadius);
		subdivide(i2, v2, v23, v12, depth-1, numverts, hemi, curtm, fRadius);
		subdivide(i3, v3, v31, v23, depth-1, numverts, hemi, curtm, fRadius);
		subdivide(i4, v12, v23, v31, depth-1, numverts, hemi, curtm, fRadius);

	}
	else // Add a triangle to the vertex array
	{
		v1.Normalize();
		v2.Normalize();
		v3.Normalize();

		// set location by vertex index and point
		if (curtm->GetStyle() & VertexPool::NORMALS)		// compute normals
		{
			*iter.GetNormal(idx[0]) = v1;
			*iter.GetNormal(idx[1]) = v2;
			*iter.GetNormal(idx[2]) = v3;
		}

		v1 *= fRadius;
		v2 *= fRadius;
		v3 *= fRadius;

		// set location by vertex index and point 
		*iter.GetLoc(idx[0]) = v1;
		*iter.GetLoc(idx[1]) = v2;
		*iter.GetLoc(idx[2]) = v3;
		if (iter.GetNormalOfs() > 3)	// locations are X,Y,Z,W?
		{
			iter.GetVtx(idx[0])[3] = 1.0f;
			iter.GetVtx(idx[1])[3] = 1.0f;
			iter.GetVtx(idx[2])[3] = 1.0f;

		}
		curtm->AddIndices(idx, 3);
	}
}


/*!
 * @fn bool GeoUtil::IcosaSphere(TriMesh* mesh, float radius, int depth, bool hemi)
 * @param mesh		mesh to add icosasphere vertices to
 * @param radius	radius of sphere
 * @param depth	 	depth of recursion to subdivide icosahedron 
 *					as approximation of a sphere
 * @param hemi		boolean flag for requesting a hemisphere (nothing below z=0)
 *					instead of a full sphere
 *
 * Adds a 3D Sphere generated from subdivision of an icosahedron 
 *
 *
 * @return  true if icosasphere successfully added, else  false
 *
 * @see GeoUtil::Ellipsoid GeoUtil::Cylinder GeoUtil::Block
 */
bool GeoUtil::IcosaSphere(TriMesh* mesh, float radius, int depth, bool hemi)
{
	static Vec3 icos_verts[12] = {
		Vec3(0.000000f, 0.000000f, 1.000000f),
		Vec3(0.000000f, 0.000000f, -1.000000f),
		Vec3(0.894427f, 0.000000f, 0.447214f),
		Vec3(0.276393f, 0.850651f, 0.447214f),
		Vec3(-0.723607f, 0.525731f, 0.447214f),
		Vec3(-0.723607f, -0.525731f, 0.447214f),
		Vec3(0.276393f, -0.850651f, 0.447214f),
		Vec3(0.723607f, 0.525731f, -0.447214f),
		Vec3(-0.276393f, 0.850651f, -0.447214f),
		Vec3(-0.894427f, 0.000000f, -0.447214f),
		Vec3(-0.276393f, -0.850651f, -0.447214f),
		Vec3(0.723607f, -0.525731f, -0.447214f)
	};

	static int32 icos_indxs[20][3] = {
		{10,1,11}, {9,1,10}, {8,1,9}, {7,1,8},
		{11,1,7}, {2,11,7}, {6,11,2}, {6,10,11},
		{5,10,6}, {5,9,10}, {4,9,5}, {4,8,9},
		{3,8,4}, {3,7,8}, {2,7,3}, {0,6,2},
		{0,5,6}, {0,4,5}, {0,3,4}, {0,2,3}
	};

	Vec3	p;
    int		i, j;

	// There are 12 vertices and 20 indices to begin with in an icosahedron 
	int		numvertices = 12, numindices = 20;
	int		numsubcalls = 1;
	int		numverts = numvertices;

	// Pre calculate the number of vertices and indices that will be needed 
	// 3 new vertices are added per subdivision call.
	// The subdivision call is recursively called 4 times until depth == 0 
	for (i = 0; i < depth; i++)
	{
		numsubcalls = 1;
		for (j = 0; j < i; j++)
			numsubcalls *= 4;
		numvertices += 20 * numsubcalls * 3; 
		numindices += 20 *numsubcalls * 3;
	}
	if (mesh->AddVertices(NULL, numvertices) < 0)
		VX_ERROR(("GeoUtil::Icosasphere out of memory\n"), false);

	// For each of the triangles in a basic icosahedron ...
	for (i = 0; i < 20; i++)
	{	
		subdivide(&icos_indxs[i][0], icos_verts[icos_indxs[i][0]],
			icos_verts[icos_indxs[i][1]], icos_verts[icos_indxs[i][2]],
			depth, &numverts, hemi, mesh, radius);	
	}
    return true;
}

}	// end Vixen
