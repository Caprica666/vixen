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

#define VX_MAX_LAYOUT_SLOTS	8


/*!
 * @class LayoutSlot
 * @brief Describes the layout of the device buffer elements.
 *
 * Each buffer element has a string name that identifies it,
 * an offset and a size. Vertex pools have two additional
 * fields giving the style of the component (what it is used for)
 * and which texture stage it belongs with (for texture coords).
 *
 * @see VertexPool::SetLayout DeviceBuffer::FindLayout VertexPool::GetLayout
 */
struct LayoutSlot
{
	Core::String	Name;		//!< name of vertex component
	int32			Style;		//!< VertexPool::NORMALS, VertexPool::COLORS, VertexPool::TEXCOORDS
	int32			Offset;		//!< offset of component in the vertex

	// For vertex buffers only
	int32			TexIndex;	//!< texture stage index
	int32			Size;		//!< number of floats/ints in this component
};

/*!
 * @class DataLayout
 * @brief Describes the in-memory layout of a device buffer.
 *
 * This structure is constructed for a buffer when it's descriptor string is provided.
 * It contains an entry for each named element giving its type, size and offset.
 * It designed to be used internally.
 *
 * @see VertexPool::SetLayout DeviceBuffer
 */
class DataLayout : public Core::BaseObj
{
public:
	DataLayout(const TCHAR* descriptor = NULL);

	DataLayout&	operator=(const DataLayout& src);

	//!< Get layout slot for named element.
	const LayoutSlot*	GetSlot(const TCHAR* name) const;

	//! Set data layout information based on string descriptor.
	void				SetDescriptor(const TCHAR* desc);

	//! Make a string descriptor from the information in the data layout.
	void				MakeDescriptor(Core::String& desc) const;

	//!< Find layout structure for given descriptor.
	static	const DataLayout*	FindLayout(const TCHAR* desc);

	//!< Free all accumulated layouts.
	static void			FreeLayouts();

	Core::String	Descriptor;		//!< layout descriptor string
	int32			NumSlots;		//!< number of vertex components
	int32			Size;			//!< size of vertex in floats
	mutable intptr	DevHandle;		//!< device specific handle

	// For vertex buffers only
	int32			NumTex;			//!< number of texture coordinate sets
	int32			Style;			//!< style of vertex
	int32			NormalOfs;		//!< offset of first normal in vertex
	LayoutSlot		Slot[VX_MAX_LAYOUT_SLOTS];

	/*
	 * Indicates whether the data in the element is integer or float.
	 * @see LayoutSlot::Style
	 */
	enum
	{
		INTEGER = 16,
		FLOAT = 0,
	};
protected:
	static void	FormatDescriptor(Core::String& outdesc, const TCHAR* indesc);

	//!< Get layout structure for given descriptor.
	static	DataLayout&	GetLayout(const TCHAR* desc)	{ Core::String s(desc); return (*s_Layouts)[s].Value; }

	static Core::Dict<Core::String, DataLayout, BaseDict>*	s_Layouts;	//!< table of vertex layouts
};

namespace Core
{
template <> inline bool Dict< Core::String, DataLayout, Vixen::BaseDict >::CompareKeys(const Core::String& knew, const String& kdict)
{
	return STRCASECMP(knew, kdict) == 0;
}

template <> inline uint32 Dict< Core::String, DataLayout, Vixen::BaseDict >::HashKey(const Core::String& np) const
{
	return HashStr((const TCHAR*) np);
}

}	// end Core

/*!
 * @class DeviceBuffer
 * @brief Encapsulates a data buffer used with a graphics display device.
 *
 * A device buffer is a fixed size data buffer with a defined format
 * that may be sent to a graphics display device, such as shader
 * constant buffers.
 *
 * @ingroup vixen
 * @see Material VertexArray
 */
class DeviceBuffer : public SharedObj
{
public:
	VX_DECLARE_CLASS(DeviceBuffer);

	//	Initializers
	DeviceBuffer(const TCHAR* desc = NULL, size_t nbytes = 0);	//!< Construct empty data buffer of the given size.
	DeviceBuffer(const DeviceBuffer& src);		//!< Construct copy of the given data buffer.
	~DeviceBuffer();

	const TCHAR*		GetDescriptor() const;	//!< Get buffer layout string descriptor.	
	const DataLayout*	GetLayout() const;		//!< Get buffer layout data descriptor.	
	size_t				GetByteSize() const;	//!< Returns buffer size in bytes.
	void*				GetData() const;		//!< Returns pointer to data area, NULL if none.
	void				SetData(void* data);	//!< Set pointer to data area.
	void				Reset(const TCHAR* desc, size_t nbytes);	//!< Reset buffer descriptor and contents.

	// Buffer data accessors
	bool			Get(const TCHAR* name, float& v) const;		//!< Get float element.
	bool			Get(const TCHAR* name, Col4& v) const;		//!< Get 3D vector element.
	bool			Get(const TCHAR* name, Vec3& v) const;		//!< Get 4D color element.
	bool			Get(const TCHAR* name, Vec4& v) const;		//!< Get 4D vector element.
	bool			Get(const TCHAR* name, Vec2& v) const;		//!< Get 2D vector element.
	bool			Set(const TCHAR* name, float v);			//!< Set float element.
	bool			Set(const TCHAR* name, int32 v);			//!< Set int element.
	bool			Set(const TCHAR* name, const Col4& v);		//!< Set 3D vector element.
	bool			Set(const TCHAR* name, const Vec3& v);		//!< Set 4D color element.
	bool			Set(const TCHAR* name, const Vec4& v);		//!< Set 4D vector element.
	bool			Set(const TCHAR* name, const Vec2& v);		//!< Set 2D vector element.
	bool			Get(const TCHAR* name, int32* dst) const;
	bool			Get(const TCHAR* name, float* dst) const;
	bool			Set(const TCHAR* name, const float* src);
	bool			Set(const TCHAR* name, const int32* src);

//  Overrides
	virtual bool	Copy(const SharedObj* src);


	enum Opcode
	{
		DEVBUF_SetLayout = SharedObj::OBJ_NextOp,
		DEVBUF_Set,
		DEVBUF_NextOp = SharedObj::OBJ_NextOp + 20,
	};

	intptr				DevHandle;

protected:
	enum
	{
		STATIC_DATA =	(1 << 11),	//!< data area is static, not allocated
	};

	void*				GetSlotData(const TCHAR* name, int32& size) const;

	const DataLayout*	m_Layout;
	size_t				m_Size;
	void*				m_Data;
};

inline size_t	DeviceBuffer::GetByteSize() const
{
	return m_Size;
}

inline void* DeviceBuffer::GetData() const
{
	return m_Data;
}

inline void DeviceBuffer::SetData(void* data)
{
	if (data)
		SetFlags(STATIC_DATA);
	else
	{
		if (!IsSet(STATIC_DATA) && m_Data)
			Core::GlobalAllocator::Get()->Free(m_Data);
		ClearFlags(STATIC_DATA);
	}
	m_Data = data;
}

inline const DataLayout* DeviceBuffer::GetLayout() const
{
	return m_Layout;
}

inline const TCHAR* DeviceBuffer::GetDescriptor() const
{
	return m_Layout->Descriptor;
}


/*!
 * @fn void* DeviceBuffer::GetSlotData(const TCHAR* name, int32& nbytes)
 * @param name		name of element to get
 * @param nbytes	number of bytes in data for this slot
 *
 * Returns a pointer to the data for the given slot.
 *
 * @returns -> slot data if successful, NULL if no such element
 *
 * @see DeviceBuffer::Get DataLayout::GetSlot
 */
inline void* DeviceBuffer::GetSlotData(const TCHAR* name, int32& nbytes) const
{
	const LayoutSlot*	slot = m_Layout->GetSlot(name);
	if (slot != NULL)
	{
		nbytes = slot->Size * sizeof(int32);
		return (float*) GetData() + slot->Offset;
	}
	nbytes = 0;
	return NULL;
}

} // end Vixen