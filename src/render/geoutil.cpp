/****
 *
 * TriMesh geometry utilities class
 * This implementation is platform-independent and is used
 * by both C and C++ bindings.
 *
 ****/
#include "vixen.h"

void PSTriMesh::CompressPrims()
{
	VXArray<PSTriPrim>	newprims(GetNumPrim());
	PSTriPrim			dstprim(GEO_None, 0); 
	int					vtxidx = 0;
	const PSTriPrim*	srcprim;

	PrimIter piter(this);
	while (srcprim = piter.Next())
	{
		if ((srcprim->Prim == dstprim.Prim) &&
			(srcprim->VtxIndex == vtxidx))
			dstprim.Size += srcprim->Size;
		else
		{
			if (dstprim.Size)
				newprims.Append(dstprim);
			dstprim = *srcprim;
			vtxidx = srcprim->VtxIndex;
		}
		vtxidx += srcprim->Size;
	   }
	if (dstprim.Size)
		newprims.Append(dstprim);
	m_Primitives.Copy(&newprims);
	Touch();
}

