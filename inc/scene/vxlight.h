/*!
 * @file vxlight.h
 * @brief Light source manipulation classes.
 *
 * Light sources illuminate objects in the scene.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxmodel.h vxmaterial.h vxscene.h
 */

#pragma once

namespace Vixen {

class LightList;
class GPULight;
class Shader;
class Renderer;

#define	LIGHT_MaxLights		16
#define	LIGHT_All			0xffff0000
#define	LIGHT_NoDevice		intptr(-1)


/*!
 * @class Light
 * @brief A light determines how the models in a scene are illuminated.
 *
 * By default, a light is located at the origin and points down
 * the negative Z axis.
 *
 * A light is a model and can be placed anywhere in a scene.
 * It can be attached to another model, so that its location and
 * orientation change with the parent model. For example,
 * you could attach two lights to a car to provide headlights
 * that move with the car. You can change the position or direction
 * of a light the same way as a model - by modifying it local matrix.
 *
 * There may be any number of lights in the scene but each light
 * adds additional calculation overhead per vertex.
 * You can use the Group::SetActive function to disable a light
 * by designating it as inactive. You can also set the light radius,
 * causing it not to illuminate objects outside this distance.
 *
 * There are several kinds of lights:  point,  directional and  spot.
 * A  directional light source acts as if it is infinitely far away
 * and produces parallel light rays, like the sun. It does not have
 * a position, only a direction. A \b point light radiates outward
 * uniformly in all directions from a specific location,
 * like a bulb or match. It has a position, but no direction.
 * A \b spot light radiates in a cone outward and has both a position
 * (the apex of the cone) and a direction (how the cone is oriented).
 * Anything outside the cone is not illuminated.
 * The Light base class behaves as a point light source.
 * Subclasses SpotLight and DirectLight provide other behavior.
 *
 * A light with no children will, by default, illuminate all objects in
 * the scene. Lights with children illuminate only their descendants
 * (all objects below them in the hierarchy). You can restrict a light
 * to illuminating all objects within a certain distance from it by
 * setting the light's illumination radius.
 *
 * @see Scene Material Model
 * @ingroup vixen
 */
class Light : public Model
{
	friend class LightList;
	friend class GPULight;
public:

	VX_DECLARE_CLASS(Light);
	Light(const TCHAR* layout_desc = TEXT("float4 WorldDir, float4 WorldPos, float4 Color, float Decay"), const TCHAR* shadername = NULL);
	~Light();

	void		SetColor(const Col4&);	//!< Set the color of the light.
	const Col4&	GetColor() const;		//!< Get the light color.
	void		SetRadius(float);		//!< Set illumination radius.
	float		GetRadius() const;		//!< Get illumination radius.
	void		SetDecay(int);			//!< Set the type of illumination decay.
	int			GetDecay() const;		//!< Get the type of illumination decay.
	void		Attach(Scene*);			//!< Attach light to a scene (internal only).
	DeviceBuffer*	GetDataBuffer();	//!< Access data buffer passed to renderer.
	const DeviceBuffer*	GetDataBuffer() const;

//	Overrides
	virtual bool	Copy(const SharedObj* src);
	virtual bool	Do(Messenger& s, int opcode);
	virtual int		Save(Messenger&, int) const;
	virtual void	SetActive(bool active);
	virtual Vec3	GetCenter(int = WORLD) const;
	virtual bool	GetCenter(Vec3*, int = WORLD) const;
	virtual void	Display(Scene*);
	virtual	void	TakeOut(Group* parent);
	virtual	void	PutIn(Group* parent);
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*!
	 * @brief Light decay values.
	 * @see SetDecay
	 */
	enum
	{
		NONE = 0,
		INVERSE = 1,
		INVERSE_SQUARE = 2
	};
   		
	/*
	 * Light::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		LIGHT_SetColor = Model::MOD_NextOp,
		LIGHT_SetOuterAngle,
		LIGHT_SetRadius,
		LIGHT_SetDecay,
		LIGHT_SetLocalDir,
		LIGHT_SetInnerAngle,
		LIGHT_Init,
		LIGHT_SetAngle = LIGHT_SetOuterAngle,
		LIGHT_NextOp = Model::MOD_NextOp + 20,
	};

public:
	mutable voidptr	DevHandle;		/* handle to device-specific stuff */

protected:
	DeviceBuffer	m_DataBuffer;	/* data passed to renderer */
	float			m_Radius;		/* illumination radius */
	Vec4			m_WorldDir;		/* start of light shader data */
	Vec4			m_WorldPos;
	Col4			m_Color;
	int				m_Decay;
};

/*!
 *@fn DeviceBuffer*	Light::GetDataBuffer()
 *
 * The device buffer associated with a light contain's the data passed
 * to the renderer each time the light is updated. Usually this data
 * is passed to the pixel shader in a constant buffer.
 *
 * @see DeviceBuffer DataLayout
 */
inline DeviceBuffer*	Light::GetDataBuffer()				{ return &m_DataBuffer; }
inline const DeviceBuffer*	Light::GetDataBuffer() const	{ return &m_DataBuffer; }

/*!
 * @class DirectLight
 * @brief A directional light source that radiates in a single direction.
 *
 * A  directional light source acts as if it is infinitely far away
 * and produces parallel light rays, like the sun. It does not have
 * a position, only a direction.
 *
 * @see Light Model SpotLight
 * @ingroup vixen
 */
class DirectLight : public Light
{
public:
	VX_DECLARE_CLASS(DirectLight);
	DirectLight() : Light() { }
	virtual Vec3	GetCenter(int = WORLD) const;
	virtual bool	GetCenter(Vec3*, int = WORLD) const;
};

/*!
 * @class AmbientLight
 * @brief An ambient light source radiates uniformly in all directions.
 *
 * An ambient light source provides uniform illumination of a specified
 * color and intensity. It does not have a position or direction.
 *
 * @see Light Model SpotLight
 * @ingroup vixen
 */
class AmbientLight : public Light
{
public:
	VX_DECLARE_CLASS(AmbientLight);
	AmbientLight() : Light() { }
	AmbientLight(const Col4& c) : Light() { SetColor(c); }
};

/*!
 * @class SpotLight
 * @brief A spot light source that radiates in a cone.
 *
 * A \b spot light radiates in a cone outward and has both a position
 * (the apex of the cone) and a direction (how the cone is oriented).
 * Anything outside the cone is not illuminated.
 *
 * @see Light Model DirectLight
 * @ingroup vixen
 */
class SpotLight : public Light
{
public:
	VX_DECLARE_CLASS(SpotLight);
	SpotLight();

	void		SetOuterAngle(float);			//!< Set spotlight cone outer angle.
	float		GetOuterAngle() const;			//!< Get spotlight cone outer angle.
	void		SetInnerAngle(float);			//!< Set spotlight cone inner angle.
	float		GetInnerAngle() const;			//!< Get spotlight cone inner angle.

	// overrides
	virtual bool	Copy(const SharedObj* src);
	virtual bool	Do(Messenger& s, int opcode);
	virtual int		Save(Messenger&, int) const;

protected:
	float			m_OuterAngle;		/* spotlight outer angle */
	float			m_InnerAngle;		/* spotlight outer angle */
};

inline SpotLight::SpotLight() : Light(TEXT("float4 WorldDir, float4 WorldPos, float4 Color, float Decay, float InnerAngle, float OuterAngle"))
{
	m_OuterAngle = PI / 3.0;
	m_InnerAngle = PI / 4.0;
	m_Decay = NONE;
}


/*!
 * @class GPULight
 * @brief Device and Scene specific light properties.
 *
 * When a light is attached to a display device, this property
 * is associated with it to hold the scene and device information.
 *
 * @see LightList Property
 * @ingroup vixenint
 * @internal
 */
class GPULight : public DeviceBuffer
{
public:
	GPULight(Renderer& render, const Light& light);
	virtual ~GPULight() { }

	Renderer&		Render;			//!< renderer we belong to
	int32			ID;				//!< light ID (index in light list)
	uint32			Mask;			//!< bit mask for light
	intptr			DevIndex;		//!< device index of light
	Vec3			LocalDir;		//!< local light direction
	const Light*	LightModel;		//!< scene light we are attached to
	int32			Type;			//!< type of light this is

	/*
	 * The phong shaders implementing lighting use these
	 * values in a constant buffer
	 */
	struct ShaderConstants
	{
		Vec4	WorldDir;		//!< light world direction
		Vec4	WorldPos;		//!< light world position
		Col4	Color;			//!< light color
		int32	Decay;			//!< how light decays with distance
		float	InnerAngle;		//!< spotlight inner angle
		float	OuterAngle;		//!< spotlight outer angle
		int32	Pad;			//!< pad to 16 byte boundary
	} Data;						//!< Data used by pixel shader

	virtual GPULight& operator=(const GPULight&);
	virtual	void		Detach()			{ }
	virtual	void		Load(bool changed)	{ }
	virtual void		Enable() const		{ }
	virtual void		Disable() const		{ }
	virtual void		Update(const Matrix* mtx);
	virtual GPULight*	Clone() const;
};


inline GPULight*	GPULight::Clone() const
{
	GPULight* lnew = new GPULight(Render, *LightModel);
	*lnew = *this;
	return lnew;
}


/*!
 * @class LightList
 * @brief Encapsulates the lights used by a scene.
 *
 * Each scene may have one or more lights from the display hierarchy
 * actively illuminating objects in that scene. Since a hierarchy
 * may be used in more than one scene, the scene-specific properties
 * of a light are stored here instead of with the light. When
 * dual-threaded rendering is used, a scene keeps two light lists
 * so a light may be active in one thread and not another.
 *
 * When a light is made  active, it is put into the light list
 * of the scene associated with the currently running thread.
 * It is attached to the display device and a property encapsulating
 * the device and scene-specific information for that light is attached to it.
 * If the light is de-activated, it marked as inactive but and detached
 * from the device but is still kept in the scene's light list.
 *
 * This behavior lets you have more lights in the scene than
 * the display hardware can support by managing them efficiently.
 * Lights with children automatically de-activate themselves until display traversal
 * For example, a light which does not have any vertices
 * within its illumination radius deactivates itself.
 *
 * @see Light Scene::Get
 * @ingroup vixenint
 * @internal
 */
class LightList : public SharedObj
{
	friend class GPULight;

public:
	/*!
	 * @class Iter
	 * @brief Iterates thru the lights for a scene.
	 *
	 * Each time LightIter::Next is called, a different
	 * light is returned. Only active lights are returned by the iterator.
	 * The same light may be used by more than one scene.
	 *
	 * @see Light LightList
	 * @ingroup vixenint
	 */
	class Iter
	{
	public:
		Iter(LightList* lights = NULL);			//!< Construct iterator for a light list.
		Iter(LightList& lights);				//!< Construct iterator for a light list.
		void		Reset(LightList* = NULL);	//!< Reset iterator for a scene.
		Light*		Next(bool& enabled);		//!< Return status for next light.
		GPULight*	NextProp(bool& enabled);	//!< Return status for next light.

	protected:
		int			m_CurLight;
		LightList*	m_Lights;
	};

	friend class Iter;

	LightList();
	LightList(const LightList& src);
	~LightList();

	LightList&		operator=(const LightList&);
	uint32	LightsActive() const	{ return m_LightsActive; }
	uint32	LightsChanged() const	{ return m_LightsChanged; }
	uint32	GetNumLights() const	{ return m_NumLights; }
	uint32	LightsEnabled() const	{ return m_LightsOn; }
	bool	CheckListChanged()		{ bool r = m_ListChanged; m_ListChanged = false; return r; }

	int		Attach(const Light*, GPULight* prop);		//!< Attach a light to the list.
	void	Detach(GPULight* prop);					//!< Detach a light property from list.
	void	DetachAll();								//!< Detach all lights from list.
	uint32	LightsOn(uint32 mask);						//!< Enable lights in the device.
	void	LoadAll();									//!< Load all lights into device.
	void	LightsOff(uint32 mask);						//!< Force off selected lights.
	void	UpdateAll();								//!< Transform lights before traversal.
	uint32	NearLights(const Model* mod, const Matrix* mtx, uint32 mask); //! Determine which lights are near a model.

	bool	DoDistanceCull;					// cull lights based on distance from object

protected:
	uint32		m_LightsActive;				// mask of lights active from traversal
	uint32		m_LightsChanged;			// mask of lights changed this frame
	int32		m_NumLights;				// number of lights used in scene
	GPULight*	m_Lights[LIGHT_MaxLights];	// light properties
	uint32		m_LightsOn;					// mask of active and enabled lights
	uint32		m_LightsOff;				// lights to keep off
	bool		m_ListChanged;				// true if items added or removed
};


} // end Vixen