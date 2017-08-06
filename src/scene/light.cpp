/****
 *
 * Core Light implementation. This code is the common functionality
 * for class Light that is shared for different ports.
 *
 ****/
#include "vixen.h"

namespace Vixen {

static const TCHAR* opnames[] =
	{ TEXT("SetColor"), TEXT("SetOuterAngle"), TEXT("SetRadius"), TEXT("SetDecay"), TEXT("SetLocalDir"), TEXT("SetInnerAngle"), TEXT("Init") };


const TCHAR** Light::DoNames = opnames;

/*
 * Maps light ID to bit mask, assumes 16 lights
 */
static	uint32	id2mask[16] = {
	0x80000000,	0x40000000,	0x20000000,	0x10000000,
	0x08000000,	0x04000000,	0x02000000,	0x01000000,
	0x00800000,	0x00400000,	0x00200000,	0x00100000,
	0x00080000,	0x00040000,	0x00020000,	0x00010000 };

VX_IMPLEMENT_CLASSID(Light, Model, VX_Light);
VX_IMPLEMENT_CLASSID(DirectLight, Light, VX_DirectLight);
VX_IMPLEMENT_CLASSID(SpotLight, Light, VX_SpotLight);
VX_IMPLEMENT_CLASSID(AmbientLight, Light, VX_AmbientLight);

Light::Light(const TCHAR* desc, const TCHAR* shadername) : Model(), m_DataBuffer(desc, 0)
{
	m_DataBuffer.SetData(&m_WorldDir);
	if (shadername)
		m_DataBuffer.SetName(shadername);
	else
		m_DataBuffer.SetName(ClassName());
	DevHandle = NULL;
	m_Radius = 0;
	m_Color.Set(1, 1, 1);
	m_Decay = Light::NONE;
	m_WorldPos.Set(0, 0, 0, 1);
	m_WorldDir.Set(0, 0, -1, 0);
	ClearFlags(MOD_BVinvalid);
	m_NoBounds = true;
	SetChanged(true);
}

Light::~Light()
{
	GPULight* prop = (GPULight*) DevHandle;
	if (prop)
	{
		prop->LightModel = NULL;
		VX_TRACE(Light::Debug, ("Light Detaching light property #%d %s\n", prop->ID, GetName()));
	}
}

/*!
 * @fn void	Light::SetRadius(float r)
 * @param r radius within which light is effective
 *
 * The radius around the light for which illumination is
 * checked within the scene graph. Objects in the scene
 * which are not within this distance from the light
 * are not illuminated by it (the light does not contribute
 * to the lighting equations for the vertices of the object).
 * This optimization is done during display traversal.
 * It is independent of the kind of light you are using.
 *
 * If a light has children, only these objects are illuminated
 * by the light. Otherwise, the light illuminates all objects in the scene.
 *
 * @see Light::SetDecay Light::SetKind
 */
void	Light::SetRadius(float v)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Light, LIGHT_SetRadius) << this << v;
	VX_STREAM_END( )

	SetChanged(true);
	m_Radius = v;
}

float Light::GetRadius() const
	{ return m_Radius; }

/*!
 * @fn void	Light::SetColor(const Col4& c)
 * @param c The color the light illuminates the scene with.
 *
 * @see Light::SeDecay
 */
void	Light::SetColor(const Col4& c)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Light, LIGHT_SetColor) << this << c;
	VX_STREAM_END( )

	SetChanged(true);
	m_Color = c;
}

const Col4& Light::GetColor() const
	{ return m_Color; }


/*!
 *@fn void	Light::SetDecay(int decay)
 * @param decay spotlight illumination decay option
 *
 * The rate of decay for the intensity of a light varies
 * with the distance from the light. It is ignored for directional lights.
 * @li Light::NONE				never decays
 * @li Light::INVERSE			decays with inverse of distance
 * @li Light::INVERSE_SQUARE	decays with inverse square of distance
 * T-
 *
 * @see Light::SetKind Light::SetRadius
 */
void	Light::SetDecay(int Decay)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Light, LIGHT_SetDecay) << this << int32(Decay);
	VX_STREAM_END( )

	SetChanged(true);
	m_Decay = Decay;
}

int Light::GetDecay() const
	{ return m_Decay; }



/*!
 * @fn void	SpotLight::SetOuterAngle(float angle)
 * @param angle  The outer angle of the cone for this spot light in radians.
 *
 * @see SpotLight::SetInnerAngle Light::SetDecay
 */
void	SpotLight::SetOuterAngle(float angle)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Light, LIGHT_SetOuterAngle) << this << angle;
	VX_STREAM_END( )

	SetChanged(true);
	m_OuterAngle = angle;
}

float SpotLight::GetOuterAngle() const
	{ return m_OuterAngle; }

/*!
 * @fn void	SpotLight::SetInnerAngle(float angle)
 * @param angle  The inner angle of the cone for this spot light in radians.
 *
 * @see SpotLight::SetOuterAngle Light::SetDecay
 */
void	SpotLight::SetInnerAngle(float angle)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Light, LIGHT_SetInnerAngle) << this << angle;
	VX_STREAM_END( )

	SetChanged(true);
	m_InnerAngle = angle;
}

float SpotLight::GetInnerAngle() const
	{ return m_InnerAngle; }

/****
 *
 * PutIn
 * Called when a light is put into a hierarchy.
 * We load the device state for the light.
 *
 ****/
void Light::PutIn(Group* parent)
{
	SetChanged(true);
	Model::PutIn(parent);
}

/****
 *
 * TakeOut
 * Called when a light is taken out of a hierarchy.
 * We remove the light from the device.
 *
 ****/
void Light::TakeOut(Group* parent)
{
	GPULight* prop = (GPULight*) DevHandle;
	if (prop)
		prop->LightModel = NULL;
	Model::TakeOut(parent);
}

/****
 *
 * Class Light override for SharedObj::Print
 *
 * Prints an ASCII description of the light
 *
 ****/
DebugOut& Light::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Model::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "\t<attr name='Color'>" << GetColor() << "</attr>");
	endl(dbg << "\t<attr name='DataBuffer'>" << m_DataBuffer.GetDescriptor() << "</attr>");
	return Model::Print(dbg, opts & PRINT_Trailer);
}

/****
 *
 * class Light override for SharedObj::Do
 *		LIGHT_SetColor	<Col4>
 *		LIGHT_SetInnerAngle	<float>
 *		LIGHT_SetOuterAngle	<float>
 *		LIGHT_SetDecay	<int32>
 *		LIGHT_SetRadius	<float>
 *
 ****/
bool Light::Do(Messenger& s, int op)
{
	float		f;
	Col4		c;
	int32		nbytes;
	Vec3		dir;
	Opcode		o = Opcode(op);
	TCHAR		buf[VX_MaxString];

	switch (op)
	{
		case LIGHT_SetColor:
		s >> c;
		SetColor(c);
		break;

		case LIGHT_SetRadius:
		s >> f;
		SetRadius(f);
		break;

		case LIGHT_SetLocalDir:
		s >> dir;
		break;

		case LIGHT_Init:
		s >> nbytes;
		s >> buf;
		m_DataBuffer.Reset(buf, nbytes);
		break;

		default:
		return Model::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Light::DoNames[op - LIGHT_SetColor]
					   << " " << this);
#endif
	return true;
}

Vec3 Light::GetCenter(int flags) const
    { return Vec3(m_WorldPos.x, m_WorldPos.y, m_WorldPos.z); }

bool Light::GetCenter(Vec3* p, int flags) const
{
	Matrix	t;

	switch (flags)
	{
		case LOCAL:
    	*p = GetTranslation();
		break;

		case WORLD:
	    TotalTransform(&t);
		*p *= t;
		break;

		default:
		break;
	}
	return true;
}

Vec3 DirectLight::GetCenter(int flags) const
    { return Vec3(0, 0, 0); }

bool DirectLight::GetCenter(Vec3* p, int flags) const
{
	if (p)
	{
		p->Set(0, 0, 0);
		return true;
	}
	return false;
}

/****
 *
 * Class Light override for SharedObj::Copy
 * 	bool Copy(SharedObj* src)
 *
 * Copies the contents of one light object into another.
 * If the source object is not a light, the attributes
 * which are light-specific are unchanged in the destination.
 *
 ****/
bool Light::Copy(const SharedObj* src_obj)
{
	const Light*	src = (const Light*) src_obj;

	ObjectLock dlock(this);
	ObjectLock slock(src);
	if (!Model::Copy(src_obj))
		return false;
	if (!src_obj->IsClass(VX_Light))
		return true;
	SetColor(src->GetColor());
	SetRadius(src->GetRadius());
	m_DataBuffer.Copy(src->GetDataBuffer());
	m_Radius = src->m_Radius;
	m_WorldPos = src->m_WorldPos;
	return true;
}


/****
 *
 * class Light override for SharedObj::Save
 *
 ****/
int Light::Save(Messenger& s, int opts) const
{
	int32 h = Model::Save(s, opts);
	if (h <= 0)
		return h;
	s << OP(VX_Light, LIGHT_Init) << h << int32(m_DataBuffer.GetByteSize()) << m_DataBuffer.GetDescriptor();
	s << OP(VX_Light, LIGHT_SetColor) << h << GetColor();
	if (GetRadius() != 0.0f)
		s << OP(VX_Light, LIGHT_SetRadius) << h << GetRadius();
	return h;
}

bool SpotLight::Do(Messenger& s, int op)
{
	int32		n;
	float		f;
	Opcode		o = Opcode(op);

	switch (op)
	{
		case LIGHT_SetOuterAngle:
		s >> f;
		SetOuterAngle(f);
		break;

		case LIGHT_SetInnerAngle:
		s >> f;
		SetInnerAngle(f);
		break;
		case LIGHT_SetDecay:
		s >> n;
		SetDecay(n);
		break;

		default:
		return Light::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Light::DoNames[op - LIGHT_SetColor]
					   << " " << this);
#endif
	return true;
}

bool SpotLight::Copy(const SharedObj* src_obj)
{
	const SpotLight*	src = (const SpotLight*) src_obj;

	ObjectLock dlock(this);
	ObjectLock slock(src);
	if (!Light::Copy(src_obj))
		return false;
	if (!src_obj->IsClass(VX_SpotLight))
		return true;
	SetOuterAngle(src->GetOuterAngle());
	SetInnerAngle(src->GetInnerAngle());
	SetDecay(src->GetDecay());
	return true;
}

int SpotLight::Save(Messenger& s, int opts) const
{
	int32 h = Light::Save(s, opts);
	if (h <= 0)
		return h;
	s << OP(VX_SpotLight, LIGHT_SetOuterAngle) << h << m_OuterAngle;
	s << OP(VX_SpotLight, LIGHT_SetDecay) << h << (int32) m_Decay;
	s << OP(VX_SpotLight, LIGHT_SetInnerAngle) << h << m_InnerAngle;
	return h;
}


LightList::Iter::Iter(LightList* lights)
{
	m_Lights = lights;
	m_CurLight = 0;
}

LightList::Iter::Iter(LightList& lights)
{
	m_Lights = &lights;
	m_CurLight = 0;
}

void LightList::Iter::Reset(LightList* lights)
{
	if (lights)
		m_Lights = lights;
	m_CurLight = 0;
}

Light* LightList::Iter::Next(bool& enabled)
{
	GPULight* prop = NextProp(enabled);
	if (prop == NULL)
		return NULL;
	return (Light*) prop->LightModel;
}

GPULight* LightList::Iter::NextProp(bool &enabled)
{
	LightList*	list = m_Lights;
	if (list == NULL)
		return NULL;
	ObjectLock	lock(list);
	while (m_CurLight <= list->m_NumLights)
	{
		GPULight* prop = list->m_Lights[m_CurLight++];
		if (prop == NULL)
			continue;
		if (!(list->m_LightsActive & prop->Mask))
			continue;
		uint32 tmp = list->m_LightsOn;
		enabled = (tmp & prop->Mask) != 0;
		return prop;
	}
	return NULL;
}

void Light::Attach(Scene* scene)
{
	Renderer*	render = scene->GetRenderer();

	if (IsActive() && (DevHandle == NULL))
	{
		render->AddLight(this);
		SetChanged(true);
	}
}

void Light::SetActive(bool active)
{
	Model::SetActive(active);
	SetChanged(true);
}

void Light::Display(Scene* scene)
{
	Attach(scene);
	if (IsParent())
	{
		bool	isactive = IsActive();

		if (!isactive)			// force stuff attached to thelight to display
			SetActive(true);
		Model::Display(scene);
		SetActive(isactive);	// restore active state
	}
	else
		Model::Display(scene);
}

LightList::LightList()
{
	DoDistanceCull = false;
	for (int i = 0; i < LIGHT_MaxLights; ++i)
		m_Lights[i] = NULL;
	m_LightsActive = 0;
	m_NumLights = 0;
	m_LightsChanged = 0;
	m_LightsOn = 0;
	m_ListChanged = true;
	m_LightsOff = uint32(-1);
}

LightList::LightList(const LightList& src)
{
	*this = src;
}

LightList::~LightList()
{
	DetachAll();
}

LightList& LightList::operator=(const LightList& src)
{
	m_ListChanged = src.m_ListChanged;
	for (int i = 0; i < LIGHT_MaxLights; ++i)
	{
		GPULight* dstprop = m_Lights[i];
		GPULight* srcprop = src.m_Lights[i];

		if (srcprop == NULL)
		{
			Detach(dstprop);
			continue;
		}
		if (dstprop == NULL)
		{
			dstprop = srcprop->Clone();
			m_Lights[i] = dstprop;
			m_ListChanged = true;
		}		
		else
			*dstprop = *srcprop;
	}
	m_LightsActive = src.m_LightsActive;
	m_LightsChanged = src.m_LightsChanged;
	m_NumLights = src.m_NumLights;
//	m_LightsOn = src.m_LightsOn;
	m_LightsOff = src.m_LightsOff;
	return *this;
}

/*!
 * @fn uint32 LightList::NearLights(const Model* mod, const Matrix* mtx, uint32 mask)
 * @param mod	model to illuminate
 * @param mtx	world matrix for model
 * @param mask	mask of lights to consider (0 indicates all lights)
 *
 * Determines which lights are near the given model.
 * Proximity is determined by the light radius. Lights
 * with a radius of zero will illuminate all models in
 * the scene. Keeps track of the number of vertices
 * lit by each light.
 *
 * @return bit mask of lights that are near the model
 *
 * @see LightList::LightsOn
 */
uint32 LightList::NearLights(const Model* mod, const Matrix* mv,uint32 mask)
{
	Sphere	mbs;
	int32	bits;
	int32	n = 0;
	Sphere	bsp;
	Vec3	ctr(mv->Get(0, 3), mv->Get(1, 3), mv->Get(2, 3));

	if (!DoDistanceCull)
		return m_LightsActive;
	if (mask == 0)
		mask = LIGHT_All;
 	mask &= m_LightsActive;
	bits = mask;
	mod->GetBound(&bsp, Model::NONE);
	while (bits)				// lights left to look at?
	{
		if (bits < 0)			// hi bit set? look at light
		{
			GPULight* prop = m_Lights[n];

			if ((prop != NULL) && (prop->LightModel != NULL))
			{
				Light* light = (Light*) prop->LightModel;
				if ((light->GetRadius() > 0) &&
					!light->IsClass(VX_DirectLight))
				{
					float d = ctr.Distance(light->GetCenter());
					if (d > light->GetRadius() + bsp.Radius)
						mask &= ~prop->Mask;		// ignore far away light
				}
			}
		}
		++n;
		bits <<= 1;
	}
	return mask;
}

/*!
 * @fn GPULight* LightList::Attach(const Light* light, GPULight* prop)
 *
 * This routine is called by the traversal or main
 * thread to attach a light to the scene if it is not already
 * in the light table. This creates a light property and attaches
 * it to the light model. The property remains attached until
 * GPULight::Detach is called.
 *
 * @see GPULight
 */
int LightList::Attach(const Light* light, GPULight* prop)
{
	ObjectLock	lock(this);
	int			i;

	for (i = 0; i < LIGHT_MaxLights; ++i)
	{
		if (m_Lights[i])
			continue;
		prop->Type = light->ClassID();
		prop->ID = i;
		prop->Mask = id2mask[i];
		prop->LightModel = light;
		m_ListChanged = true;
		m_Lights[i] = prop;
		m_LightsChanged |= prop->Mask;
		light->DevHandle = prop;
		++m_NumLights;
		VX_TRACE(Light::Debug, ("LightList::Attach #%d %s\n", i, light->GetName()));
		return i;
	}
	VX_ERROR(("Light::Attach cannot attach light %d\n", i), NULL);
}

/*!
 * @fn void LightList::DetachAll()
 * This routine is used when you are changing scene graphs or during
 * shutdown to detach the light properties from the light objects.
 * It does not garbage collect the light properties so they can
 * be reused again.
 *
 * @see GPULight::Detach
 */
void LightList::DetachAll()
{
	if (m_NumLights <= 0)
		return;
	ObjectLock	lock(this);
	for (int i = 0; i < LIGHT_MaxLights; ++i)
	{
		GPULight* prop = m_Lights[i];

		if (prop)
		{
			Detach(prop);
			m_Lights[i] = NULL;
			VX_TRACE(Light::Debug, ("LightList::DetachAll deleting light %d\n", i));
		}
	}
	m_LightsActive = 0;
	m_NumLights = 0;
	m_ListChanged = true;
}

void LightList::LightsOff(uint32 mask)
{
	m_LightsOff = mask;
}

/*!
 * @fn uint32 LightList::LightsOn(uint32 mask)
 * Enables the lights specified by the input mask to be
 * used for rendering the next shape. This routine is only
 * called from rendering thread.
 *
 * @see LightList::NearLights
 */
uint32 LightList::LightsOn(uint32 mask)
{
	ObjectLock	lock(this);
	int32		bits ;
	int			i = -1;

	mask &= m_LightsOff;					// force these lights always off
	bits = mask ^ m_LightsOn;				// mask of lights to change
	m_LightsOn = mask;
	while (bits && (++i < m_NumLights))		// lights that changed
	{
		if (bits < 0)						// hi bit set? update light
		{
			GPULight*	prop = m_Lights[i];
			if ((prop == NULL) || (prop->DevIndex == LIGHT_NoDevice))
				continue;
			if (mask & 0x80000000)
				prop->Enable();
			else prop->Disable();
		}
		bits <<= 1;
		mask <<= 1;
	}
	return m_LightsOn;		// remember active ones
}

/*!
 * @fn void LightList::UpdateAll()
 *
 * Transforms the centers of all the active lights
 * into world coordinates. This routine is only
 * called from the display traversal thread.
 *
 * @see LightList::LoadAll Light::Transform
 */
void LightList::UpdateAll()
{
	ObjectLock	lock(this);
	uint32		lightson = m_LightsOff & m_LightsActive;
	Matrix		mtx;

	m_LightsChanged = 0;
	for (int i = 0; i < LIGHT_MaxLights; ++i)
	{
		GPULight*	prop = m_Lights[i];
		Light*		l;
		Vec3		v;
		int32		mask;
		int			changed;

		if (prop == NULL)
			continue;
		l = (Light*) prop->LightModel;
		if ((l == NULL) && (prop->DevIndex != LIGHT_NoDevice))
		{
			Detach(prop);
			continue;
		}
		mask = prop->Mask;
		if (l->IsActive())
		{
			if ((m_LightsActive & mask) == 0)
				m_LightsChanged |= mask;
			m_LightsActive |= mask;
		}
		else
		{
			if (m_LightsActive & mask)
				m_LightsChanged |= mask;
			m_LightsActive &= ~mask;
			continue;
		}
		changed = l->HasChanged();
		l->TotalTransform(&mtx);
		mtx.GetTranslation(v);
		if (v.DistanceSquared((Vec3&) l->m_WorldPos) >= VX_EPSILON)
			changed = true;
		l->m_WorldPos = v;
		mtx.TransformVector(prop->LocalDir, v);
		if (v.DistanceSquared((Vec3&) l->m_WorldDir) >= VX_EPSILON)
			changed = true;
		l->m_WorldDir = v;
		if (changed)
		{
			prop->Update(&mtx);
			m_LightsChanged |= mask;
			l->SetChanged(false);
		}
	}
}


/*!
 * @fn void LightList::LoadAll()
 * Loads the positions and directions of all light sources
 * into the device. If the light has changed, other attributes
 * are refreshed as well.
 *
 * This routine is only called from the rendering thread.
 * It does not access the original Light objects.
 *
 * @see LightList::TransformAll LightList::DetachAll
 */
void LightList::LoadAll()
{
	ObjectLock	lock(this);

	for (int i = 0; i < LIGHT_MaxLights; ++i)
	{
		GPULight* lprop = m_Lights[i];

		if (lprop && (lprop->DevIndex != LIGHT_NoDevice))
		{
			int32 mask = lprop->Mask;

			if (lprop->LightModel == NULL)		// light has been deleted?
			{
				lprop->Disable();
				Detach(lprop);
			}
			else if (m_LightsActive & mask)		// light is active
			{
				if (m_LightsChanged & mask)
					lprop->Load((m_LightsChanged & mask) != 0);
				lprop->Enable();
			}
			else if (m_LightsOn & mask)			// not active but currently enabled
			{
				m_LightsOn = m_LightsOn & ~mask;// disable the light
				lprop->Disable();
			}
		}
	}
}

GPULight::GPULight(Renderer& render, const Light& light)
  :	Render(render),
	DeviceBuffer(light.GetDataBuffer()->GetDescriptor(),
				 (light.GetDataBuffer()->GetByteSize() <= sizeof(ShaderConstants)) ? 0 : light.GetDataBuffer()->GetByteSize())
{
	DevIndex = LIGHT_NoDevice;
	ID = -1;
	Mask = 0;
	LightModel = NULL;
	LocalDir.Set(0, 0, -1);
	if (m_Data == NULL)
		SetData(&Data);
	Type = 0;
	Data.Color.Set(1, 1, 1);
	Data.WorldDir.Set(0, 0, -1, 0);
	Data.WorldPos.Set(0, 0, 0, 0);
	Data.InnerAngle = 0;
	Data.OuterAngle = 0;
}

GPULight& GPULight::operator=(const GPULight& src)
{
	DeviceBuffer::operator=(src);
	Render = src.Render;
	LocalDir = src.LocalDir;
	ID = src.ID;
	Type = src.Type;
	Mask = src.Mask;
	DevIndex = src.DevIndex;
	return *this;
}

void GPULight::Update(const Matrix* mtx)
{
	Vixen::Light*	l = (Vixen::Light*) LightModel;
	DeviceBuffer*	lbuf = l->GetDataBuffer();
	size_t			nbytes = lbuf->GetByteSize();

	lbuf->Set(TEXT("WorldPos"), l->m_WorldPos);
	lbuf->Set(TEXT("WorldDir"), l->m_WorldDir);
	VX_ASSERT(GetByteSize() >= nbytes);
	memcpy(GetData(), lbuf->GetData(), nbytes);
}

void LightList::Detach(GPULight* prop)
{
	ObjectLock	lock(this);
	int			mask = prop->Mask;
	Light*		light = (Light*) prop->LightModel;

	VX_ASSERT(prop->ID >= 0);
	m_Lights[prop->ID] = NULL;
	m_ListChanged = true;
	if (light)
	{
		prop->LightModel = NULL;
		light->DevHandle = NULL;
		VX_TRACE(Light::Debug, ("LightList::Detach #%d %s\n", prop->ID, light->GetName()));
	}
	if (prop->DevIndex != LIGHT_NoDevice)
	{
		prop->Disable();
		prop->Detach();
	}
	m_LightsActive &= ~mask;
	m_LightsOn &= ~mask;
	--m_NumLights;
}

}	// end Vixen