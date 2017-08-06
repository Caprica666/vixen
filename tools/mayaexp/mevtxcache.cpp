#include "vix_all.h"
  
meVtxCache::meVtxCache(Mesh* mesh) :  Dictionary< Vec3, int >()
{
	m_CompareSize = 0;
	if (mesh)
		SetMesh(mesh);
}

void meVtxCache::SetMesh(Mesh* mesh)
{
	m_Mesh = mesh;
	m_Iter.Init(mesh->GetVertices());
	if (m_CompareSize == 0)
		m_CompareSize = mesh->GetVtxSize();
}

bool meVtxCache::CompareVertex(const float* vtx2, int index)
{
	float* vtx1 = m_Iter.GetVtx(index);

    if (vtx1 == NULL)
		return false;
	for (int i = 0; i < m_CompareSize; ++i)
		if (*vtx1++ != *vtx2++)
			return false;
    return true;
}

int meVtxCache::AddVertex(const float* vtxptr)
{
    Vec3	key(vtxptr[0], vtxptr[1], vtxptr[2]);
    int		nverts = GetNumVtx();

	VertexArray* verts = GetVertices();
    Set(key, nverts);
	if (verts->AddVertices(vtxptr, 1) == nverts)
	{
		Set(key, nverts);
		return nverts;
	}
	meLog(1, "ERROR: %s out of memory, cannot add vertex");
	return -1;	
}

int meVtxCache::FindVertex(const float* vtxptr)
{
    Entry*	entry;
    Vec3	key(vtxptr[0], vtxptr[1], vtxptr[2]);
    int		index = HashKey(key) & m_Mask;

    for (entry = (Entry*) m_Buckets[index];     // search all entries
         entry != NULL;                         // in our bucket
         entry = (Entry*) entry->Next)
    {   
        int vtxidx = entry->Value;              // index of this vertex in geoarray
        if (CompareVertex(vtxptr, vtxidx))		// found it?
			return vtxidx;						// return its index
    }
    return AddVertex(vtxptr);					// make it a new one
}
