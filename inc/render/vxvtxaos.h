#pragma once

namespace Vixen {

/*!
 * @class VertexArray
 * @brief Dynamically growing array of vertices.
 *
 * This implementation organizes the vertices as an array of structures, keeping
 * all vertex data in a single contiguous chunk of memory.
 *
 * A vertex may contain a location (Vec3),
 * a normal (Vec3), a color (Col4) and texture
 * coordinates (Vec2). The components, if present,
 * are always stored contiguously in the above order.
 * The vertex layout determines which components are present.
 * All of the vertices in a VtxArray have the same layout.
 * If you change the layout, the data in the vertex array is unchanged
 * but will be interpreted differently.
 *
 * @see VertexPool Mesh TriMesh
 */
class VertexArray : public VertexPool
{
public:
	VX_DECLARE_CLASS(VertexArray);

	VertexArray(int style = VertexPool::NORMALS, intptr size = 0);
	VertexArray(const TCHAR* layout_desc, intptr size = 0);
	VertexArray(const VertexArray&);

//!	Retrieve device-dependent pointer to vertex data information
	virtual	const float*	GetData() const;
	virtual	float*	GetData();
	virtual	bool	SetMaxVtx(intptr size);
	virtual	bool	SetNumVtx(intptr size);
	virtual	intptr	AddVertices(const float* floatArray, intptr size);
	virtual	float*	PadVertices(const float* floatArray, intptr size, intptr srcstride);
	virtual	bool	Copy(const SharedObj*);

	mutable voidptr	DevHandle;	// device-dependent handle
	static	Core::Allocator*	VertexAlloc;

protected:
	FloatArray		m_Data;		// vertex data
};

} // end Vixen