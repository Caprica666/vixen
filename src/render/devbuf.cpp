/****
 *
 * Material Class
 *
 ****/
#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASS(DeviceBuffer, SharedObj);

Core::Dict<Core::String, DataLayout, BaseDict>*	DataLayout::s_Layouts = NULL;

DataLayout::DataLayout(const TCHAR* descriptor)
{
	NumSlots = 0;
	NormalOfs = -1;
	NumTex = 0;
	Size = 0;
	DevHandle = NULL;
	Style = 0;
	if (descriptor)
		SetDescriptor(descriptor);
}

void DataLayout::FormatDescriptor(Core::String& outdesc, const TCHAR* indesc)
{
	const TCHAR* p = indesc;
	int			size;
	TCHAR		name[128];
	TCHAR		type[64];

	while (*p && ((*p == TEXT(' ')) || (*p == TEXT('\t'))))
		++p;
	if ((*p == TEXT('i')) || (*p == TEXT('f')))
	{
		outdesc = p;
		return;
	}
	outdesc.Empty();
	while (SSCANF(p, TEXT("%s %s %d"), name, type, &size) == 3)
	{
		if (!outdesc.IsEmpty())
			outdesc += TEXT(", ");
		outdesc += type;
		if (size > 1)
			outdesc += Core::String(size);
		outdesc += TEXT(' ');
		outdesc += name;
		while (*p && (*p++ != ','))
			;
	}
}

void DataLayout::SetDescriptor(const TCHAR* descriptor)
{
	const TCHAR*	p = descriptor;
	int				nslots = 0;
	int				ofs = 0;

	FormatDescriptor(Descriptor, descriptor);
	NumSlots = 0;
	NormalOfs = -1;
	NumTex = 0;
	Style = 0;
	if (p == 0)
		return;
	while (*p)
	{
		LayoutSlot&	slot = Slot[NumSlots];
		int			size;
		TCHAR		name[128];
		TCHAR*		q = name;

		slot.Style = 0;
		if (STRNCMP(p, TEXT("int"), 3) == 0)
		{
			slot.Style |= DataLayout::INTEGER;
			p += 3;
		}
		else if (STRNCMP(p, TEXT("float"), 5) == 0)
			p += 5;
		else
			break;
		if (SSCANF(p, TEXT("%d"), &size))
		{
			while (*p &&
					(*p != TEXT(' ')) &&
					(*p != TEXT('\t')) &&
					(*p != TEXT(',')))
				p++;
		}
		else
			size = 1;
		while (*p &&
				(*p == TEXT(' ')) ||
				(*p == TEXT('\t')) ||
				(*p == TEXT(',')))
			p++;
		while (*p &&
				(*p != TEXT(' ')) &&
				(*p != TEXT('\t')) &&
				(*p != TEXT(',')))
			*q++ = *p++;
		*q = 0;
		slot.Name = name;
		slot.Offset = Size;
		slot.TexIndex = -1;
		slot.Size = size;		// parse the size
		++NumSlots;
		Size += slot.Size;
		VX_ASSERT(slot.Size >= 1);
		while (*p &&
				(*p == TEXT(' ')) ||
				(*p == TEXT('\t')) ||
				(*p == TEXT(',')))
			p++;

	}
}

/*!
 * @fn int DataLayout::GetSlot(const TCHAR* name) const
 * @param name	name of element whose index we want
 *
 * Finds the layout slot for the element in the DataLayout for this buffer.
 * @returns -> layout slot of element or NULL if not found
 *
 * @see DataLayout::FindLayout
 */

const LayoutSlot* DataLayout::GetSlot(const TCHAR* name) const
{
	for (int i = 0; i < NumSlots; ++i)
		if (Slot[i].Name == name)
			return &Slot[i];
	return NULL;
}


/*!
 * @fn const DataLayout* DataLayout::FindLayout(const TCHAR* layout_desc)
 * @param layout_desc	string describing layout
 *
 * Finds a buffer layout based on its descriptor.
 * The layout descriptor is a string that
 * Establishes the layout of the buffer elements.
 * Each element has a name, a type and a size
 * indicating the number of 32 bit words it occupies.
 *
 * For example, a vertex with locations, normals and two sets
 * of 2D texture coordinates might be described as:
 *		position float 3, normal float 3, texcoord0 float 2, texcoord1 float 2
 * A material with two colors would look like:
 *		firstcolor float 3, secondcolor float 3
 *
 * The layout descriptor is used as a hash key to uniquely
 * identify a particular buffer layout. If the layout has already
 * been used, this function returns the existing one.
 * Otherwise, it makes a new layout descriptor and returns that one.
 *
 * @see VertexPool::SetLayout DeviceBuffer::DeviceBuffer
 */
const DataLayout* DataLayout::FindLayout(const TCHAR* layout_desc)
{
	Core::String	temp;
	int				nslots = 0;

	FormatDescriptor(temp, layout_desc);
	if (s_Layouts == NULL)
	{
		s_Layouts = new Core::Dict<Core::String, DataLayout, BaseDict>();
		s_Layouts->IncUse();
	}
	else				// is layout already there? return it
	{
		const DataLayout* layout = s_Layouts->Find(temp);
		if (layout)
			return layout;
	}
	DataLayout&		newlayout = (*s_Layouts)[temp].Value;
	newlayout.SetDescriptor(temp);
	return &newlayout;
}

/*!
 * @fn void DataLayout::FreeLayouts()
 *
 * All DataLayout objects are kept in a static array (s_Layouts).
 * This function frees all the DataLayout objects.
 * It should only be called on shutdown.
 */
void DataLayout::FreeLayouts()
{
	if (s_Layouts)
	{
		s_Layouts->Empty();
		s_Layouts->Delete();
		s_Layouts = NULL;
	}
}

/*!
 * @fn  void DataLayout::MakeDescriptor(Core::String& desc) const
 * @param desc	String to get the layout descriptor
 *
 * The DataLayout constructor populates the members of the
 * object based on the descriptor string. This function
 * is the inverse - it creates a descriptor string based on
 * the data in the other members of the DataLayout.
 *
 * @see DataLayout::DataLayout
 */
void DataLayout::MakeDescriptor(Core::String& desc) const
{
	desc = "";
	for (int i = 0; i < NumSlots; ++i)
	{
		if (!desc.IsEmpty())
			desc += TEXT(", ");
		if (Slot[i].Style & DataLayout::INTEGER)
			desc += TEXT("int");
		else
			desc += TEXT("float");
		if (Slot[i].Size > 1)
			desc += Core::String(Slot[i].Size);
		desc += TEXT(" ") + Slot[i].Name;
	}
}

DataLayout&	DataLayout::operator=(const DataLayout& src)
{
	Descriptor = src.Descriptor;
	NumSlots = src.NumSlots;
	Size = src.Size;
	DevHandle = src.DevHandle;
	NumTex = src.NumTex;
	Style = src.Style;
	NormalOfs = src.NormalOfs;
	for (int i = 0; i < src.NumSlots; ++i)
		Slot[i] = src.Slot[i];
	return *this;
}

/*!
 * @fn DeviceBuffer::DeviceBuffer(const TCHAR* desc)
 * @param desc	Buffer layout descriptor.
 *
 * Establishes the layout of the buffer elements.
 * Each component has a name, a type, and a size indicating
 * the number of 32 bit words it occupies.
 * The layout description is a string that looks like:
 *	slotname1 type1 size1, slotname2 type2 size2,
 *
 * For example, a buffer with a color and an intensity
 * might be described as:
 *	color float 3, intensity float 1
 *
 * @see DeviceBuffer::SetDiffuse
 */
DeviceBuffer::DeviceBuffer(const TCHAR* desc, size_t nbytes)
{
	m_Data = NULL;
	m_Size = (int32) 0;
	DevHandle = 0;
	Reset(desc, nbytes);
}

DeviceBuffer::DeviceBuffer(const DeviceBuffer& src)
{
	DevHandle = 0;
	m_Size = 0;
	m_Data = NULL;
	Copy(&src);
}

void	DeviceBuffer::Reset(const TCHAR* desc, size_t nbytes)
{
	const DataLayout*	layout = m_Layout;

	if (desc)
	{
		m_Layout = DataLayout::FindLayout(desc);
		m_Size = m_Layout->Size * sizeof(float);
		m_Size = (m_Size + 15) & ~0X0F;			// 16 byte boundary
	}
	else
	{
		m_Layout = NULL;
		m_Size = 0;
	}
	if (nbytes > 0)
	{
		m_Size = (nbytes + 15) & ~0X0F;			// 16 byte boundary
		SetData(Core::GlobalAllocator::Get()->Alloc(nbytes));
	}
}

DeviceBuffer::~DeviceBuffer()
{
	if (m_Data && !IsSet(STATIC_DATA))
		Core::GlobalAllocator::Get()->Free(m_Data);
}

bool DeviceBuffer::Copy(const SharedObj* src_obj)
{
	const DeviceBuffer*	src = (const DeviceBuffer*) src_obj;

	ObjectLock dlock(this);
	ObjectLock slock(src);
	if (!SharedObj::Copy(src_obj))
		return false;
	if (src->IsKindOf(CLASS_(DeviceBuffer)))
	{
		m_Layout = src->m_Layout;
		if (!IsSet(STATIC_DATA))
		{
			if (m_Data)
				Core::GlobalAllocator::Get()->Free(m_Data);
			if (m_Layout && (m_Layout->Size > 0))
				m_Data = Core::GlobalAllocator::Get()->Alloc(m_Layout->Size);
		}
	}
	return true;
}


/*!
 * @fn bool DeviceBuffer::Set(const TCHAR* name, float* src)
 * @param name	name of element to set
 * @param src	-> floating values for this element.
 *
 * Changes the value of a named element in the buffer.
 * This function assumes the source pointer references a data
 * of the same size as the buffer element!
 *
 * @returns true if successful, false if no such element
 *
 * @see DeviceBuffer::Get DataLayout::GetSlot
 */
bool DeviceBuffer::Set(const TCHAR* name, const float* src)
{
	int32	nbytes;
	float*	data = (float*) GetSlotData(name, nbytes);

	if (data)
	{
		memcpy(data, src, nbytes);
		return true;
	}
	return false;
}

/*!
 * @fn bool DeviceBuffer::Set(const TCHAR* name, int32* src)
 * @param name	name of element to set
 * @param src	-> integer values for this element.
 *
 * Changes the value of a named element in the buffer.
 * This function assumes the source pointer references a data
 * of the same size as the buffer element!
 *
 * @returns true if successful, false if no such element
 *
 * @see DeviceBuffer::Get DataLayout::GetSlot
 */
bool DeviceBuffer::Set(const TCHAR* name, const int32* src)
{
	int32	nbytes;
	int32*	data = (int32*) GetSlotData(name, nbytes);

	if (data)
	{
		memcpy(data, src, nbytes);
		return true;
	}
	return false;
}

/*!
 * @fn bool DeviceBuffer::Set(const TCHAR* name, const Col4& c)
 * @param name	name of element to set
 * @param c		new value for color
 *
 * Assigns a color into the named element.
 * @returns true if successful, false if no such element or size mismatch
 *
 * @see DeviceBuffer::Get
 */
bool DeviceBuffer::Set(const TCHAR* name, const Col4& c)
{
	int32	nbytes;
	Col4*	data = (Col4*) GetSlotData(name, nbytes);

	if (data)
	{
		VX_ASSERT(nbytes == sizeof(Col4));
		*data = c;
		return true;
	}
	return false;
}

/*!
 * @fn bool DeviceBuffer::Set(const TCHAR* name, const Vec4& v)
 * @param name	name of element to set
 * @param v		new value for vector
 *
 * Assigns a 4 component vector into the named element.
 * @returns true if successful, false if no such element or size mismatch
 *
 * @see DeviceBuffer::Get
 */
bool DeviceBuffer::Set(const TCHAR* name, const Vec4& v)
{
	int32	nbytes;
	Vec4*	data = (Vec4*) GetSlotData(name, nbytes);

	if (data)
	{
		VX_ASSERT(nbytes == sizeof(Vec4));
		*data = v;
		return true;
	}
	return false;

}

/*!
 * @fn bool DeviceBuffer::Set(const TCHAR* name, const Vec3& v)
 * @param name	name of element to set
 * @param v		new value for vector
 *
 * Assigns a 3 component vector into the named element.
 * @returns true if successful, false if no such element or size mismatch
 *
 * @see DeviceBuffer::Get
 */
bool DeviceBuffer::Set(const TCHAR* name, const Vec3& v)
{
	int32	nbytes;
	Vec3*	data = (Vec3*) GetSlotData(name, nbytes);

	if (data)
	{
		VX_ASSERT(nbytes == sizeof(Vec3));
		*data = v;
		return true;
	}
	return false;
}

/*!
 * @fn bool DeviceBuffer::Set(const TCHAR* name, const Vec2& v)
 * @param name	name of element to set
 * @param v		new value for vector
 *
 * Assigns a 2 component vector into the named element.
 * @returns true if successful, false if no such element or size mismatch
 *
 * @see DeviceBuffer::Get
 */
bool DeviceBuffer::Set(const TCHAR* name, const Vec2& v)
{
	int32	nbytes;
	Vec2*	data = (Vec2*) GetSlotData(name, nbytes);

	if (data)
	{
		VX_ASSERT(nbytes == sizeof(Vec2));
		*data = v;
		return true;
	}
	return false;
}


/*!
 * @fn bool DeviceBuffer::Set(const TCHAR* name, float v)
 * @param name	name of element to set
 * @param v		new value for element
 *
 * Assigns a scalar value into the named element.
 * @returns true if successful, false if no such element or size mismatch
 *
 * @see DeviceBuffer::Get
 */
bool DeviceBuffer::Set(const TCHAR* name, float v)
{
	int32	nbytes;
	float*	data = (float*) GetSlotData(name, nbytes);

	if (data)
	{
		VX_ASSERT(nbytes == sizeof(float));
		*data = v;
		return true;
	}
	return false;
}

/*!
 * @fn bool DeviceBuffer::Set(const TCHAR* name, int32 v)
 * @param name	name of element to set
 * @param v		new value for element
 *
 * Assigns a scalar value into the named element.
 * @returns true if successful, false if no such element or size mismatch
 *
 * @see DeviceBuffer::Get
 */
bool DeviceBuffer::Set(const TCHAR* name, int32 v)
{
	int32	nbytes;
	int32*	data = (int32*) GetSlotData(name, nbytes);

	if (data)
	{
		VX_ASSERT(nbytes == sizeof(int32));
		*data = v;
		return true;
	}
	return false;
}

/*!
 * @fn bool DeviceBuffer::Get(const TCHAR* name, float& v)
 * @param name	name of element to get
 * @param v		gets the value of the element
 *
 * Retrieves a scalar element by name.
 * @returns true if successful, false if no such element or size mismatch
 *
 * @see DeviceBuffer::Get
 */
bool DeviceBuffer::Get(const TCHAR* name, float& v) const
{
	int32	nbytes;
	float*	data = (float*) GetSlotData(name, nbytes);

	if (data)
	{
		v = *data;
		return true;
	}
	return false;
}

/*!
 * @fn bool DeviceBuffer::Get(const TCHAR* name, float* dst) const
 * @param name	name of element to set
 * @param src	-> where to store floating values for this element.
 *
 * Copies the value of a named element in the buffer.
 * This function assumes the destination pointer references a data
 * of the same size as the buffer element!
 *
 * @returns -> true if successful, false if no such element
 *
 * @see DeviceBuffer::Set DataLayout::GetSlot
 */
bool DeviceBuffer::Get(const TCHAR* name, float* dst) const
{
	int32	nbytes;
	Vec3*	data = (Vec3*) GetSlotData(name, nbytes);

	if (data)
	{
		memcpy(dst, data, nbytes);
		return true;
	}
	return false;
}

/*!
 * @fn bool DeviceBuffer::Get(const TCHAR* name, int32* dst) const
 * @param name	name of element to set
 * @param dst	-> where to store integer values for this element.
 *
 * Copies the value of a named element in the buffer.
 * This function assumes the destination pointer references a data
 * of the same size as the buffer element!
 *
 * @returns -> true if successful, false if no such element
 *
 * @see DeviceBuffer::Set DataLayout::GetSlot
 */
bool DeviceBuffer::Get(const TCHAR* name, int32* dst) const
{
	ObjectLock	lock(this);
	int32		nbytes;
	void*		data = GetSlotData(name, nbytes);

	if (data)
	{
		memcpy(dst, data, nbytes);
		return true;
	}
	return false;
}

/*!
 * @fn bool DeviceBuffer::Get(const TCHAR* name, Col4& c) const
 * @param name	name of element to set
 * @param c		gets the value for the color
 *
 * Gets a color from the named element.
 * @returns true if successful, false if no such element or size mismatch
 *
 * @see DeviceBuffer::Get
 */
bool DeviceBuffer::Get(const TCHAR* name, Col4& c) const
{
	ObjectLock	lock(this);
	int32		nbytes;
	Col4*		data = (Col4*) GetSlotData(name, nbytes);

	if (data)
	{
		VX_ASSERT(nbytes == sizeof(Col4));
		c = *data;
		return true;
	}
	return false;
}

/*!
 * @fn bool DeviceBuffer::Get(const TCHAR* name, Vec4& v) const
 * @param name	name of element to set
 * @param v		gets value of vector element
 *
 * Gets a 4 component vector from the named element.
 * @returns true if successful, false if no such element or size mismatch
 *
 * @see DeviceBuffer::Get
 */
bool DeviceBuffer::Get(const TCHAR* name, Vec4& v) const
{
	ObjectLock	lock(this);
	int32		nbytes;
	Vec4*		data = (Vec4*) GetSlotData(name, nbytes);

	if (data)
	{
		VX_ASSERT(nbytes == sizeof(Vec4));
		v = *data;
		return true;
	}
	return false;

}

/*!
 * @fn bool DeviceBuffer::Get(const TCHAR* name, Vec3& v) const
 * @param name	name of element to get
 * @param v		gets value of vector element
 *
 * Gets a 3 component vector from the named element.
 * @returns true if successful, false if no such element or size mismatch
 *
 * @see DeviceBuffer::Get
 */
bool DeviceBuffer::Get(const TCHAR* name, Vec3& v) const
{
	ObjectLock	lock(this);
	int32		nbytes;
	Vec3*		data = (Vec3*) GetSlotData(name, nbytes);

	if (data)
	{
		VX_ASSERT(nbytes == sizeof(Vec3));
		v = *data;
		return true;
	}
	return false;

}

/*!
 * @fn bool DeviceBuffer::Get(const TCHAR* name, Vec2& v) const
 * @param name	name of element to get
 * @param v		gets value of vector element
 *
 * Gets a 2 component vector from the named element.
 * @returns true if successful, false if no such element or size mismatch
 *
 * @see DeviceBuffer::Get
 */
bool DeviceBuffer::Get(const TCHAR* name, Vec2& v) const
{
	ObjectLock	lock(this);
	int32		nbytes;
	Vec2*		data = (Vec2*) GetSlotData(name, nbytes);

	if (data)
	{
		VX_ASSERT(nbytes == sizeof(Vec2));
		v = *data;
		return true;
	}
	return false;
}

}	// end Vixen