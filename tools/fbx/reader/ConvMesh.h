#pragma once

namespace Vixen { namespace FBX {

// Save mesh vertices, normals, UVs and indices in GPU with OpenGL Vertex Buffer Objects
class ConvMesh : public ConvModel
{
public:
    ConvMesh(FbxNode* pNode);
	Model*			MakeShapes(const TCHAR* filebase);
	VertexArray*	MakeVerts(const TCHAR* vtxdesc = NULL);
	IntArray*		GetVertexMap()	{ return mVertexCache.GetVertexMap(); }
	VertexArray*	GetVertices()	{ return mVertices; }

protected:
	void			CopyVerts(VertexArray* vtxarray, const FbxGeometryElementNormal* normal_elem, const FbxGeometryElementUV* uv_elem);
	void			OptimizeVerts(VertexArray* vtxarray, FbxLayerElementArrayTemplate<int>* materialMap, FbxGeometryElement::EMappingMode materialMapMode);

	const FbxMesh*		mMesh;
	VertexArray*		mVertices;
	PtrArray			mShapes;
	VertexCache			mVertexCache;
    bool				mHasNormal;
    bool				mHasUV;
    bool				mAllByControlPoint; // Save data in VBO by control point or by polygon vertex.
};


// Property cache, value and animation curve.
struct PropertyChannel
{
    PropertyChannel() : mAnimCurve(NULL), mValue(0.0f) {}
    // Query the channel value at specific time.
    GLfloat Get(const FbxTime & pTime) const
    {
        if (mAnimCurve)
        {
            return mAnimCurve->Evaluate(pTime);
        }
        else
        {
            return mValue;
        }
    }

    FbxAnimCurve * mAnimCurve;
    GLfloat mValue;
};

} }	// end FBX