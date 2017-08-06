/*!
 * @file vxinterpolator.h
 * @brief Time based interpolation classes
 *
 * These classes produce output values which vary over time.
 * They use a set of input keys and linearly interpolate between
 * them based on the current time.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxkeyframer.h vxxformer.h
 */
#pragma once

namespace Vixen {

/*!
 * @class Evaluator
 * @brief Base class for a collection of engines that varies a value over time.
 *
 * The  value is a fixed size array of floating point numbers which
 * is recomputed every frame by the evaluator.
 *
 * Sometimes it is useful to combine the outputs of multiple evaluators.
 * Each evaluator has an  alpha value that can be used for blending
 * output values with the destination. If the alpha is 1, the evaluator
 * replaces the destination value with the computed one. Otherwise, the computed
 * value is multiplied by  alpha and added to the destination.
 *
 * The default implementation for Evaluator produces the elapsed time
 * as its evaluation result.
 * 
 * @see KeyFramer Transformer Interpolator Engine::GetElapsed Engine::GetDuration
 */
class Evaluator : public Engine
{
public:
	/*!
	 * @brief weighted quaternion.
	 *
	 * Quaternion with an extra weight component used internally
	 * when blending rotations from different animations.
	 */
	class WQuat : public Quat
	{
	public:
		WQuat() { weight = 0.0f; }
		WQuat(float X, float Y, float Z, float W, float mW = 0.0f)
		{
			x = X;
			y = Y;
			z = Z;
			w = W;
			weight = mW;
		}

		WQuat& operator=(const WQuat& src)
		{
			x = src.x;
			y = src.y;
			z = src.z;
			w = src.w;
			weight = src.weight;
			return *this;
		}

		WQuat& operator=(const Quat& src)
		{
			x = src.x;
			y = src.y;
			z = src.z;
			w = src.w;
			weight = 0.0f;
			return *this;
		}

		//--- data member ---
		float   weight;
	};
	/*!
	 * @brief Destination type.
	 * @see SetDestType
	 */
	enum
	{
		UNKNOWN = 0,
		POSITION,
		ROTATION,
		SCALE,
		LOOK_ROTATION,
		LOCAL_POSITION,
		ALPHA,
		XPOS,
		YPOS,
		ZPOS,
		COLOR,
		FACE,
		LAST_DEST_TYPE = FACE
	};

	/*!
	 * @brief Interpolation type.
	 * @see SetInterpType
	 */
	enum
	{
		STEP = 0,		//!< step between successive values - no interpolation
		LINEAR = 1,		//!< linear interpolation between values
		QSTEP = 4,		//!< weighted blending of quaternions.
		SLERP = 5,		//!< spherical linear interpolation of quaternions.
		MAX_KEY_SIZE = 16
	};

	VX_DECLARE_CLASS(Evaluator);

	Evaluator();
	int				GetDestType() const;		//!< Return destination type.
	void			SetDestType(int t);			//!< Set destination type.
	int				GetValSize() const;			//!< Return number of floats per value.
	void			SetValSize(int size);		//!< Set number of floats per value.
	float*			GetDest() const;			//!< Get pointer to destination value.
	void			SetDest(float *INOUT);		//!< Set destination value pointer.
	float			GetAlpha() const;			//!< Get alpha value for interpolation.
	void			SetAlpha(float alpha);		//!< Set alpha value for interpolation.
	virtual void	ClearDest();				//!< Clear destination value
	virtual void	SetResult(const float* INPUT);	//!< Set initial result vector.
	virtual const float* GetResult() const;			//!< Get pointer to local result vector.

	// internal overrides
	virtual bool	OnStart();
	virtual bool	Eval(float t);
	virtual bool	Copy(const SharedObj*);
	virtual bool	Do(Messenger& s, int op);
	virtual int		Save(Messenger&, int) const;
	virtual DebugOut&	Print(DebugOut& dbg, int opts) const;

	enum Opcode
	{
		INTERP_SetSize = Engine::ENG_NextOp,	// was KF_SetSize in V1
		INTERP_SetKeys,
		INTERP_SetValSize,
		INTERP_SetMaxSize,
		INTERP_SetDestType,
		INTERP_SetInterpType,
		INTERP_SetAlpha,
		INTERP_SetMaxOrder,
		INTERP_AddKey,
		INTERP_ScaleKeys,
		INTERP_NextOp = Engine::ENG_NextOp + 20
	};

protected:
	//! return pointer to computed value for given input time.
	virtual const float* ComputeValue(float t);
 
	//! blend input value with destination using interpolator alpha
	virtual bool   BlendEval(const float *v);

	int32	m_ValSize;
	int32	m_DestType;
	float	m_Alpha;
	float*	m_Dest;
};

inline int  Evaluator::GetDestType() const
{ return m_DestType; }

inline int  Evaluator::GetValSize() const
{ return m_ValSize; }

inline float* Evaluator::GetDest() const
{ return m_Dest; }

inline float  Evaluator::GetAlpha() const
{ return m_Alpha; }


/*!
 * @class Vec3Blender
 * @brief blends 3 component vector values
 */
class Vec3Blender : public Evaluator
{
public:
	Vec3Blender() : Evaluator() { m_XInput = m_YInput = m_ZInput = NULL; }
	const float*	GetXInput() const	{ return m_XInput; }
	const float*	GetYInput() const	{ return m_YInput; }
	const float*	GetZInput() const	{ return m_ZInput; }
	void			SetXInput(const float* v)	{ m_XInput = v; }
	void			SetYInput(const float* v)	{ m_YInput = v; }
	void			SetZInput(const float* v)	{ m_ZInput = v; }
	void			SetResult(const float* r);

	virtual const float*	GetResult() const;
	virtual bool			OnStart();

protected:
	const float*	ComputeValue(float t);

	Vec3			m_Result;	// result vector
	const float*	m_XInput;	// -> X input track
	const float*	m_YInput;	// -> Y input track
	const float*	m_ZInput;	// -> Z input track
};

/*!
 * @class Interpolator
 * @brief interpolates between a set of input keys to produce the output value
 *
 * The  value is a fixed size array of floating point numbers which
 * is recomputed every frame by the interpolator.
 *
 * The input keys used to compute the value are also kept in an array
 * of floats. How the keys are stored and used to compute the value is usually
 * defined by subclasses. The default implementation performs a linear,
 * interpolation between input keys which are assumed to be uniformly
 * spaced in time.
 *
 * \b KeyFramer is a subclass that keeps a time for each key and interpolates between
 * the two keys before and after the current time.
 *
 * @see Evaluator KeyFramer Transformer
 */
class Interpolator : public Evaluator
{
public:
	VX_DECLARE_CLASS(Interpolator);

	Interpolator();
	void			SetInterpType(int t);	//!< Set interpolation type
	int				GetInterpType() const;	//!< Get interpolation type
	virtual int		GetSize() const;		//!< Return number of keys.
	virtual void	SetSize(int size);		//!< Set number of keys.
	virtual int		GetMaxSize() const;		//!< Return maximum keys we have room for.
	virtual void	SetMaxSize(int size);	//!< Set size of key data storage.
	virtual float	GetTime(int i) const;	//!< Return time based on key index.

	//! Add key and time to the data area.
	virtual bool	AddKey(float time, const float* floatArray);

	//! Get key data based on index.
	virtual const float* GetKey(int i) const;

	//! Set the data array to use for key storage.
	void	SetKeys(const FloatArray* keydata);

	//! Get data array for key storage. NULL if no keys.	
	const FloatArray*	GetKeys() const;

	//! Select keys to interpolate between.
	virtual bool		SelectKeys(float& t1, float& t2, const float** key1, const float** key2);

	//! Apply a scale factor to the all the key values, times are unaffected.
	virtual	void		ScaleKeys(float scale_factor);

	// internal overrides
	void				ClearDest();
	virtual bool		Copy(const SharedObj*);
	virtual bool		Do(Messenger& s, int op);
	virtual int			Save(Messenger&, int) const;
	virtual DebugOut&	Print(DebugOut& dbg, int opts) const;
	virtual const float*	GetResult() const;

protected:
	const float* ComputeValue(float t);

	//! Perform weighted linear interpolation between the given values.
	bool		LinearEval(const float *v1, const float *v2, float w);

	//! Perform spherical interpolation between quaternions.
	bool		SlerpEval(const float *q1, const float *q2, float w);

	//! Blend between destination and input quaterion
	bool			QuatBlend(const Quat* q);

	int32			m_InterpType;
	int32			m_LastKey;
	Ref<FloatArray>	m_Keys;
};

inline int  Interpolator::GetInterpType() const
{ return m_InterpType; }

inline const FloatArray* Interpolator::GetKeys() const
{ return m_Keys; }

} // end Vixen