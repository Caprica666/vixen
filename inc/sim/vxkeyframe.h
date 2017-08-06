/*!
 * @file vxkeyframe.h
 * @brief Key framer interpolation classes.
 *
 * Keyframers produce output vectors which vary over time.
 * They use a set of key frames for input and interpolate between
 * them to produce position and rotation tracks for animation.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxinterpolator.h vxxformer.h
 */
#pragma once

namespace Vixen {

/*!
 * @class KeyFramer
 * @brief Engine that interpolates a value over time based on a set of key frames provided.
 *
 * The keyframer stores a set of time-stamped keys. The key frames are kept
 * in the order of increasing time. The  value of the keyframer is
 * obtained by interpolating between two of these keys.
 *
 * The keyframer engine is the backbone of the scene manager animation support.
 * A combination of Transformer and KeyFramer engines are made by the 3D Studio
 * MAX Exporter when transform animation controllers are used.
 *
 * @see Transformer Interpolator Interpolator Evaluator
 */
class KeyFramer : public Interpolator
{
public:
	VX_DECLARE_CLASS(KeyFramer);

	KeyFramer();

	int		GetPart() const;		//!< Get part indicator.
	void	SetPart(int part);		//!< Set part indicator.
	void	SetSize(int size);
	void	SetMaxSize(int size);
	float	GetTime(int i) const;
	bool	AddKey(float time, const float* floatArray);
	const float* GetKey(int i) const;

	//! Search for a key based on its value.
	int		Find(const float* val, float dist = 0);

	//--- overrides ---
	virtual bool	OnStart();
	virtual bool	Copy(const SharedObj*);
	virtual bool	Do(Messenger& s, int op);
	virtual int		Save(Messenger&, int) const;

	//! Called to choose the two interpolation keys.
	bool		SelectKeys(float& t1, float& t2, const float** key1, const float** key2);

	enum Opcode
	{
		KEY_SetMaxSize = INTERP_NextOp,
		KEY_AddKey,
		KEY_SetSize,
		KEY_SetPart,
		KEY_NextOp = INTERP_NextOp + 20,
	};

protected:
	int32		m_Part;				// part of target being interpolated
};

/*!
 * @class Key
 *
 * @brief base template class for keyframers of different types.
 *
 * @param CLASS	class of key and value of keyframer
 * This template class allows you to make keyframers
 * which have input keys of a given type, such as vector or color.
 * It restricts the keys and values to be of the same class.
 */
template <class CLASS> class Key : public KeyFramer
{
public:
	Key<CLASS>();

	const CLASS&	GetVal(int i) const;
	bool			AddVal(float t, const CLASS& key);
	const float*	GetResult() const;
protected:
	CLASS			m_Result;
};

template <class V> inline const float* Key<V>::GetResult() const
{
	return (const float*) &m_Result;
}

template <class V> inline Key<V>::Key() : KeyFramer()
{
	int8 s = int8(sizeof(V) / sizeof(float));
	m_ValSize = s;
}

template <class V> inline const V& Key<V>::GetVal(int i) const
{
	return *((const V*) KeyFramer::GetKey(i));
}

template <class V> inline bool Key<V>::AddVal(float t, const V& val)
{
	return KeyFramer::AddKey(t, (const float*) &val);
}

/*!
 * @class Interp
 * @brief base template class for value-specific interpolators.
 *
 * @param CLASS	class of key and value of being interpolated
 * This template class allows you to make interpolators
 * which have input keys of a given type, such as vector or color.
 * It restricts the keys and values to be of the same class.
 */
template <class CLASS> class Interp : public Interpolator
{
public:
	Interp<CLASS>();

	const CLASS&	GetVal(int i) const;
	bool			AddVal(float t, const CLASS& key);
	const float*	GetResult() const;
protected:
	CLASS			m_Result;
};


template <class V> inline Interp<V>::Interp() : Interpolator()
{
	int8 s = int8(sizeof(V) / sizeof(float));
	m_ValSize = s;
}

template <class V> inline const V& Interp<V>::GetVal(int i) const
{
	return *((const V*) Interpolator::GetKey(i));
}

template <class V> inline const float* Interp<V>::GetResult() const
{
	return (const float*) &m_Result;
}

template <class V> inline bool Interp<V>::AddVal(float t, const V& val)
{
	return Interpolator::AddKey(t, (const float*) &val);
}


/*!
 * @class KeyBlender
 * @brief interpolates between keyframers
 *
 * Key blenders can be used to blend two actively running keyframe
 * animations. Typically, the bone animator is used to merge two
 * hierarchies for blending by inserting key blenders where needed.
 *
 * You can also use a key blender to blend from the current state
 * to the start of a child keyframe animation. A key blender with
 * no children is still useful - it will interpolate between the
 * current state and a specified key.
 *
 * The default is to blend between the first two active interpolator children.
 * You can also blend between the current state and an individual keyframer.
 * You cannot blend between keyframers that are not direct descendants.
 *
 * Blending is performed by linearly interpolating the alpha value of
 * the keyblender over the blend duration. This alpha selects the
 * percentage of the keyframes blended.
 *
 * - Engine::BLEND_BETWEEN
 *		blends between the first two active interpolator children.
 *		if there is only one active child, it will provide the
 *		value and no blending will occur. default blend period
 *		is from now until child keyframers stop.
 * - Engine::BLEND_TO
 *		blends between the current state and the start of the
 *		first active interpolator child. If there is no active
 *		child, blending is between the current state and the
 *		stored key value. default blend period is from now
 *		until start time of first active child.
 *
 * The  duration of the keyblender controls how long the blending period
 * will be. If the duration is zero, no blending occurs at all and the
 * child keyframers operate independently. A positive duration specifies
 * the blend period. A duration of -1 will default the blend time depending
 * on the type of blending chosen. 
 * 
 * @see Transformer Interpolator KeyFramer BoneAnimator Engine::SetControl Engine::SetDuration
 */
class KeyBlender : public Evaluator
{
public:
	VX_DECLARE_CLASS(KeyBlender);

	KeyBlender();

	virtual bool			Eval(float t);
	virtual bool			OnStart();
	virtual bool			OnStop();
	virtual const float*	ComputeValue(float t);
	void					Blend(float time, int blendopts = Engine::BLEND_BETWEEN);
	virtual void			SetResult(const float* v);
	virtual const float*	GetResult() const;
	virtual void			ClearDest();
	virtual	float			ComputeTime(float t);

	enum Opcode
	{
		KEY_Blend = Evaluator::INTERP_NextOp,
		KEYBLEND_NextOp = Evaluator::INTERP_NextOp + 10,
	};

protected:
	Evaluator*	FindInterp(Engine*);
	bool		Setup(float t);

	float	m_Result[MAX_KEY_SIZE];
};

#ifdef SWIG
typedef Key< Col4 >		KeyColor;
%template(KeyColor)		Key<Col4>;
typedef Key< floa t>	FloatInterp;
%template(FloatInterp)	Key< float >;
#endif

/*!
 * @class ColorInterp
 * @brief interpolates a color over time based on a set
 * of key frames (times and colors).
 *
 * If the target of the color interpolator is a scene manager object that 
 * contains colors (material, light, scene, fog) one of the object colors is interpolated.
 * The Interpolator::SetPart function establishes which part of the target
 * is affected by the interpolator. These are the legal values:
 *
 * @code
 *	Part				Target		Description
 *	MAT_SetDiffuse		Material	Interpolates material diffuse color (default)
 *	MAT_SetAmbient		Material	Interpolates material ambient color
 *	MAT_SetSpecular		Material	Interpolates material specular color
 *	MAT_SetEmission		Material	Interpolates material emissive color
 *	LIGHT_SetColor		Light		Interpolates light color (default)
 *	SCENE_SetAmbient	none		Interpolates scene ambient light color (default)
 *	SCENE_SetBackColor	none		Interpolates scene background color
 *	0					none		No interpolation
 * @endcode
 *
 * @see KeyFramer Interpolator Col4 Material::SetDiffuse Light::SetColor
 */
class ColorInterp : public Key<Col4>
{
public:
	VX_DECLARE_CLASS(ColorInterp);

	ColorInterp() : Key<Col4>()
		{ 	SetDestType(COLOR); SetDest((float*) &m_ColorResult); }

	virtual bool	Eval(float t);

protected:
	Col4	m_ColorResult;
};

/*!
 * @class TimeInterp
 *
 * @brief Engine that produces a time track based on a set of key frames.
 *
 * The key frames in the time interpolator let you map the current
 * time into a different time which is used to evaluate all child engines.
 * This allows you to control the animation sequence of the child engines by
 * manipulating their input times.
 *
 * @see KeyFramer Interpolator
 */
class TimeInterp : public Key<float>
{
public:
	VX_DECLARE_CLASS(TimeInterp);
	TimeInterp();

	bool		OnStart();
	void		ComputeChildren(float, int filter = 0);
protected:
	float		m_TimeResult;
	float		m_TimeScale;
};

} // end Vixen