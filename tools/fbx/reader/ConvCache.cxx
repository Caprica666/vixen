#include "vixen.h"
#include "ConvNode.h"
#include "ConvMesh.h"
#include "ConvAnim.h"
#include "FBXReader.h"

namespace Vixen { namespace FBX {


MeshAnimator*	ConvMeshAnim::MakeMeshAnim()
{
	MeshAnimator*	deform = new MeshAnimator();
	m_VixNode = deform;
	OpenCache();
	return deform;
}

SharedObj*	ConvMeshAnim::Convert()
{
	if (ConvNode::Convert())
	{
		ReadCache();
		CloseCache();
	}
	return m_VixNode;
}

/*
 * Prepares Vertex Cache data to be read from external cache files.
 * 
 */
bool ConvMeshAnim::OpenCache()
{
	FbxCache* cache = mCacheDeformer->GetCache();
	FbxStatus status;

	if ((cache == NULL) || !mCacheDeformer->IsActive())
		return false;
	if (cache->IsOpen())
		return true;
/*
 * 3D Studio Max Point Cache
 */
	if (cache->GetCacheFileFormat() == FbxCache::eMaxPointCacheV2)
	{
#if 0
		if (!cache->ConvertFromPC2ToMC(FbxCache::eMCOneFile, FBXReader::FrameRate, FbxCache::eMCX, &status))
			VX_ERROR(("ERROR: %s cannot convert MAX cache  %s\n", m_Name, status.GetErrorString()), false);
#endif
	}
/*
 * Maya Point Cache
 */
	else if (cache->GetCacheFileFormat() == FbxCache::eMayaCache)
	{
		if (!cache->ConvertFromMCToPC2(FBXReader::FrameRate, 0, &status))
			VX_ERROR(("ERROR: %s cannot convert Maya vertex cache  %s\n", m_Name, status.GetErrorString()), false);
	}
/*
 * Try to open the Vertex Cache file
 */
	if (!cache->OpenFileForRead(&status))
	{
		mCacheDeformer->SetActive(false);
		VX_ERROR(("ERROR: %s cannot open vertex cache file %s\n", m_Name, status.GetErrorString()), false);
	}
	return true;
}

/*
 * Reads the data from the vertex cache file corresponding to this mesh animation.
 * The cache contains samples of the geometry at each time step. It is converted to
 * a sequence of vertex arrays which become the sources for the Vixen MeshAnimator.
 *
 * @return number of vertex arrays produced
 */
int ConvMeshAnim::ReadCache()
{
	FbxCache*	cache = mCacheDeformer->GetCache();
	int			n;

	if ((cache == NULL) || ((n = cache->GetChannelCount()) == 0))
		return 0;
	for (int i = 0; i < n; ++i)	// search for float vector or double vector channels only
	{
		FbxCache::EMCDataType type;
		
		cache->GetChannelDataType(i, type);
		if ((type == FbxCache::eDoubleVectorArray) ||
			(type == FbxCache::eFloatVectorArray))
			return ReadCacheChannel(i);
	}
	return 0;
}

/*
 * Reads the channel from the vertex cache file corresponding to this mesh animation.
 * The channel contains samples of the geometry at each time step. It is converted to
 * a sequence of vertex arrays which become the sources for the Vixen MeshAnimator.
 *
 * @return number of vertex arrays produced
 */
int ConvMeshAnim::ReadCacheChannel(int channel)
{
	FbxCache*		cache = mCacheDeformer->GetCache();
	MeshAnimator*	meshanim = (MeshAnimator*) m_VixNode;
	FbxTime			start;
	FbxTime			stop;
	VertexArray*	vtxarray;
	int				frame = 0;
	FbxTime			inc;
	Core::String	name(m_Name);

	name += TEXT('-');
	cache->GetChannelSamplingRate(channel, inc);
	meshanim->SetTimeInc((float) inc.GetSecondDouble());
	cache->GetAnimationRange(channel, start, stop);
	for (FbxTime t = start; t <= stop; t += inc)
	{
		vtxarray = ReadCacheSample(channel, t);
		vtxarray->SetName(name + Core::String(frame));
		meshanim->SetSource(frame, vtxarray);
	}
	VX_TRACE(FBXReader::Debug, ("%s: Sampled %d frames from vertex cache\n", m_Name, frame));
	return frame;
}

/*
 * Reads a sample from the Vertex Cache for this mesh animation.
 * A single sample corresponds to the positions of the geometry at a particular time.
 * In Vixen, this is represented as a single vertex array.
 * @param channel	index of channel to read
 * @param time		time of the sample to read
 *
 * @return Vixen VertexArray containing points for the sample at the given time, NULL on error
 */
VertexArray* ConvMeshAnim::ReadCacheSample(int channel, FbxTime time)
{
	FbxCache*		cache = mCacheDeformer->GetCache();
	int				n = cache->GetDataCount(channel);
	VertexArray*	vtxarray = new VertexArray(TEXT("float3 position"), n);
	FbxCache::EMCDataType type;
	
	cache->GetChannelDataType(channel, type);
	if (type ==	FbxCache::eDoubleVectorArray)
	{
		double*				data = (double*) Core::GlobalAllocator::Get()->Alloc(n * 3 * sizeof(double));
		VertexArray::Iter	iter(vtxarray);
		Vec3*				dstptr;
		double*				srcptr = data;
	
		if (!cache->Read(channel, time, data, n))
			VX_WARNING(("ERROR: %s could not read vertex cache data\n", m_Name));
		vtxarray->SetMaxVtx(n);
		vtxarray->SetNumVtx(n);
		while (dstptr = (Vec3*) iter.Next())
		{
			dstptr->x = (float) *srcptr++;
			dstptr->y = (float) *srcptr++;
			dstptr->z = (float) *srcptr++;
		}
		Core::GlobalAllocator::Get()->Free(data);
	}
	else if (type == FbxCache::eFloatVectorArray)
	{
		float*	data = (float*) Core::GlobalAllocator::Get()->Alloc(n * 3 * sizeof(float));
		
		if (!cache->Read(channel, time, data, n))
			VX_WARNING(("ERROR: %s could not read vertex cache data\n", m_Name));
		vtxarray->SetMaxVtx(n);
		vtxarray->SetNumVtx(0);
		vtxarray->AddVertices(data, n);
		Core::GlobalAllocator::Get()->Free(data);
	}
	else
	{
		VX_ERROR(("ERROR: %s unsupported vertex cache channel type\n", m_Name), NULL);
	}
	return vtxarray;
}

void ConvMeshAnim::CloseCache()
{
	FbxCache* cache = mCacheDeformer->GetCache();

	if (cache)
		cache->CloseFile();
}

} }	// end FBX