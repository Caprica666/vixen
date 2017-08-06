// file: interpolator.cpp

#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Evaluator, Engine, VX_Evaluator);
VX_IMPLEMENT_CLASSID(Interpolator, Evaluator, VX_Interpolator);

static const TCHAR* opnames[] =
{
	TEXT("SetSize"),
	TEXT("SetKeys"),
	TEXT("SetValSize"),
	TEXT("SetMaxSize"),
	TEXT("SetDestType"),
	TEXT("SetInterpType"),
	TEXT("SetMaxOrder"),
	TEXT("SetAlpha"),
	TEXT("AddKey")
};

const TCHAR** Evaluator::DoNames = opnames;

const TCHAR** Interpolator::DoNames = opnames;

Evaluator::Evaluator() : Engine()
{
	//--- defaults to linear blending of single float values ---
	m_ValSize    = 1;
	m_Dest       = NULL;
	m_Alpha      = 1.0f;
	m_DestType	 = UNKNOWN;
}

/*!
 * @fn const float* Evaluator::ComputeValue(float t)
 *
 * Returns a pointer to the result vector for this evaluator.
 * The base evaluator provides no result storage - subclasses
 * are responsible for the format and computation of their results.
 * The result for the base evaluator is the elapsed time.
 *
 * @see Engine::SetDuration Engine::GetElapsed Evaluator::ComputeValue
 */
const float* Evaluator::GetResult() const
{	return &m_Elapsed;	}

/*!
 * @fn const float* Evaluator::ComputeValue(float t)
 *
 * Computes the value of this evaluator for the given time.
 * Usually, the value is stored in a local result vector.
 * The base evaluator provides no result storage - subclasses
 * are responsible for the format and computation of their results.
 *
 * The result for the base evaluator is the elapsed time.
 *
 * @see Engine::SetDuration Engine::GetElapsed
 */
const float* Evaluator::ComputeValue(float t)
{	return  &m_Elapsed; }

void Evaluator::SetResult(const float*) { }

/*!
 * @fn void   Evaluator::SetDestType(int type)
 * @param type destination value type
 *
 * This parameter describes the type and size of the value computed
 * by this interpolator and stored at the destination pointer.
 * The interpolator does not use this parameter internally
 * but it can be queried by engines that wish to use interpolators to compute
 * input values for them. For example, you can use one interpolator to compute
 * the alpha value for another.
 * @code
 *	UNKNOWN			destination value not updated
 *	POSITION		updates Transformer position
 *	ROTATION		updates Transformer rotation
 *	SCALE			updates Transformer scale
 *	LOOK_ROTATION	updates Transformer look rotation
 *	ALPHA			updates Evaluator alpha value
 * @endcode
 *
 * @see Evaluator::SetDest Interpolator::SetInterpType Transformer
 */
void   Evaluator::SetDestType(int type)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Evaluator, INTERP_SetDestType) << this << int32(type);
	VX_STREAM_END( )

	m_DestType = type;
}


/*!
 * @fn void   Evaluator::SetValSize(int size)
 * @param size number of floats in the output value
 *
 *	This is the size of the destination value computed in floats.
 *	For example, points have 3 floats, quaterions have 4.
 *
 * @see Evaluator::SetKeyType Evaluator::SetDest SVec3 Quat
 */
void   Evaluator::SetValSize(int size)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Evaluator, INTERP_SetValSize) << this << int32(size);
	VX_STREAM_END(  )

	m_ValSize = size;
}


/*!
 * @fn void   Evaluator::SetDest(float *dest)
 * @param dest pointer to where to store destination value
 *
 * If the input pointer is non-zero, the interpolated value
 * computed each frame will be stored there.
 * It is assumed that the input array has room for at least
 * the number of floats specified by SetValSize;
 *
 * @see Evaluator::SetValSize
 */
void   Evaluator::SetDest(float *dest)
{
	m_Dest = dest;
}

void Evaluator::ClearDest()
{
	memset(m_Dest, 0, m_ValSize * sizeof(float));
}

/*!
 * @fn void   Evaluator::SetAlpha(float alpha)
 * @param alpha interpolation value between 0 and 1
 *
 * Used for barycentric blending of multiple interpolators.
 * If the value is one, the output values are written directly
 * into the destination. Otherwise, the output values are multiplied by
 * the alpha and added into the destination.
 *
 * @see Evaluator::SetDest Evaluator::SetDestType
 */
void   Evaluator::SetAlpha(float alpha)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Evaluator, INTERP_SetAlpha) << this << alpha;
	VX_STREAM_END(  )

	m_Alpha = alpha;
}

/*!
 * @fn bool Evaluator::OnStart()
 *
 * Set the destination of all evaluator children to be the
 * current result of this evaluator. This will cause the children
 * to compute a weighted sum of their inputs and store this
 * in the result of this evaluator before the local Eval is called.
 *
 * @see Evaluator::Eval Evaluator::GetResult
 */
bool Evaluator::OnStart()
{
	GroupIter<Engine> iter(this, Group::CHILDREN);
	Evaluator*	e;
	
	while (e = (Evaluator*) iter.Next())
	{
		if (!e->IsClass(VX_Evaluator) || (e->GetDestType() == 0))
			continue;
		e->SetDest((float*) GetResult());
		if (GetValSize() == 0)
			SetValSize(e->GetValSize());
		if (GetDestType() == 0)
			SetDestType(e->GetDestType());
		VX_ASSERT(GetValSize() == e->GetValSize());
	}
	return true;
}

bool Evaluator::Copy(const SharedObj* srcobj)
{
	ObjectLock dlock(this);
	ObjectLock slock(srcobj);
	if (!Engine::Copy(srcobj))
		return false;

	if (srcobj->IsClass(VX_Interpolator))
	{
		const Evaluator* src = (const Evaluator*) srcobj;
		m_ValSize    = src->m_ValSize;
		m_Dest       = src->m_Dest;
		m_DestType   = src->m_DestType;
		m_Alpha      = src->m_Alpha;
	}
	return true;
}

bool Interpolator::Copy(const SharedObj* srcobj)
{
	ObjectLock dlock(this);
	ObjectLock slock(srcobj);
	if (!Evaluator::Copy(srcobj))
		return false;

	if (srcobj->IsClass(VX_Interpolator))
	{
		const Interpolator* src = (const Interpolator*) srcobj;
		m_InterpType = src->m_InterpType; 
		m_Keys		 = src->m_Keys;
	}
	return true;
}

/*!
 * @fn bool Evaluator::Eval(float t)
 *
 * Performs a weighted linear interpolation between the destination
 * value and the next input value.
 *
 * The input time is zero when the interpolator is started and is measured
 * in seconds. Each time the interpolator resets, the time goes to zero again.
 *
 * @see Evaluator::SetAlpha Evaluator::SetDest
 */
bool Evaluator::Eval(float t)
{
	const float* v = ComputeValue(t);

	if (v == NULL)
		return true;
	Engine *par = (Engine*) Parent();
	if (par)
		par->SetChanged(true);
	return BlendEval(v);
}


bool Evaluator::BlendEval(const float *v)
{
	float* result = (float*) GetResult();
	int	i;

	if (m_Dest == NULL)
		return true;
	if (result)
		if (m_Alpha == 1.0f)
			for (i = 0; i < m_ValSize; i++)
			{
				result[i] = v[i];
				m_Dest[i] = v[i];
			}
		else
			for (i = 0; i < m_ValSize; i++)
			{
				result[i] = v[i];
				m_Dest[i] += m_Alpha * result[i];
			}
	else
		if (m_Alpha == 1.0f)
			for (i = 0; i < m_ValSize; i++)
				m_Dest[i] = v[i];
		else
			for (i = 0; i < m_ValSize; i++)
				m_Dest[i] += m_Alpha * v[i];
	return true;
} 

bool Evaluator::Do(Messenger& s, int op)
{
	int32	n;
	float	f;

	switch (op)
	{
		case INTERP_SetDestType:
		s >> n;
		SetDestType(n);
		break;

		case INTERP_SetValSize:
		s >> n;
		SetValSize(n);
		break;

		case INTERP_SetAlpha:
		s >> f;
		SetAlpha(f);
		break;

		default:
		return Engine::Do(s,op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Evaluator::DoNames[op - INTERP_SetSize]
					   << " " << this);
#endif
	return true;
}

bool Interpolator::Do(Messenger& s, int op)
{
	float  f;
	int32  n;
	float	key[MAX_KEY_SIZE];
	SharedObj*	obj;

	switch (op)
	{
		case INTERP_SetInterpType:
		s >> n;
		SetInterpType(n);
		break;

		case INTERP_SetKeys:
		s >> obj;
		VX_ASSERT(!obj || obj->IsClass(VX_Array));
		if ((GetValSize() == 5) && (GetDestType() == ROTATION))
		{
			const FloatArray*	src = (const FloatArray*) obj;
			intptr		nkeys = src->GetSize() / 5;
			FloatArray* keys = new FloatArray(4 * nkeys);

			keys->SetSize(4 * nkeys);
			for (intptr i = 0; i < nkeys; ++i)
				for (int j = 0; j < 4; ++j)
					keys->SetAt(4 * i + j, src->GetAt(5 * i + j));
			SetValSize(4);
			SetKeys(keys);
		}
		else
			SetKeys((FloatArray*) obj);
		break;

		case INTERP_SetMaxSize:
		s >> n;
		SetMaxSize(n);
		break;

		case INTERP_SetSize:
		s >> n;
		SetSize(n);
		break;

		case INTERP_AddKey:
		s >> f;
		s.Input(key, m_ValSize);
		AddKey(f, key); 
		break;

		default:
		return Evaluator::Do(s,op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Evaluator::DoNames[op - INTERP_SetSize]
					   << " " << this);
#endif
	return true;
}


int Evaluator::Save(Messenger& s, int opts) const
{
	int32 h = Engine::Save(s, opts);
	if (h <= 0)
		return h;

	s << OP(VX_Evaluator, INTERP_SetValSize) << h << (int32) GetValSize();
	s << OP(VX_Evaluator, INTERP_SetAlpha) << h << GetAlpha();
	if (m_DestType)
		s << OP(VX_Evaluator, INTERP_SetDestType) << h << (int32) GetDestType();
	return h;
}

int Interpolator::Save(Messenger& s, int opts) const
{
	int32 h = Evaluator::Save(s, opts);
	const FloatArray* keys = GetKeys();

	if (h <= 0)
		return h;
	s << OP(VX_Interpolator, INTERP_SetInterpType) << h << (int32) GetInterpType();
    if (keys && (keys->Save(s, opts) >= 0) && h)
		s << OP(VX_Interpolator, INTERP_SetKeys) << h << keys;
	return h;
}

Interpolator::Interpolator() : Evaluator()
{
 	m_InterpType = LINEAR;
	m_LastKey = 0;
};

/*!
 * @fn void   Interpolator::SetInterpType(int type)
 * @param type destination value type
 *
 * Describes the type of interpolation performed.
 * @code
 *	Interpolator::NONE		destination value not updated
 *	Interpolator::STEP		use current key
 *	Interpolator::LINEAR	linear interpolation between keys
 *	Interpolator::SLERP		spherical linear interpolation between keys
 * @endcode
 *
 * @see Evaluator::SetDest Interpolator::SetInterpType Transformer
 */
void   Interpolator::SetInterpType(int type)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Interpolator, INTERP_SetInterpType) << this << int32(type);
	VX_STREAM_END( )

	m_InterpType = type;
}


/*!
 * @fn void   Interpolator::SetKeys(const FloatArray* keydata)
 * @param keydata data array containing key information
 *
 * The format of the key data array is not defined by the base interpolator.
 * Selection of the keys to interpolate between is defined by SelectKeys
 * and can be overridden by subclasses.
 *
 * @see Interpolator::GetKeys Interpolator::SelectKeys KeyFramer
 */
void   Interpolator::SetKeys(const FloatArray* keydata)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Interpolator, INTERP_SetKeys) << this << keydata;
	VX_STREAM_END(  )

	m_Keys = keydata;
}

/*!
 * @fn void Interpolator::SetSize(int size)
 * @param size number of keyframes requested
 *
 * Establishes the number of key frames currently stored.
 * The maximum size can be set to any value and the storage area
 * will grow dynamically as needed.
 *
 * @see Array::SetSize Interpolator::SetMaxSize
 */
void Interpolator::SetSize(int size)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Interpolator, INTERP_SetSize) << this << int32(size);
	VX_STREAM_END( )

	if (GetKeys())
		m_Keys->SetSize(size * m_ValSize);
	else if (size)
		m_Keys = new FloatArray(size * m_ValSize);
}

int Interpolator::GetSize() const
{
	intptr n;

	if (GetKeys() == NULL)
		return 0;
	n = m_Keys->GetSize() / m_ValSize;
	VX_ASSERT(n < INT_MAX);
	return (int) n;
}


/*!
 * @fn void Interpolator::SetMaxSize(int size)
 * @param size new maximum
 *
 * Establishes the maximum number of key frames which can be stored.
 * The internal arrays will grow dynamically as needed to accomodate more keys.
 * If a size of zero is given, the internal data structures are freed.
 * They will be allocated again when new keys are added.
 *
 * @see ByteArray::SetMaxSize Interpolator::SetSize Interpolator::AddKey
 */
void Interpolator::SetMaxSize(int size)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Interpolator, INTERP_SetMaxSize) << this << int32(size);
	VX_STREAM_END( )

	if (size == 0)
	{
		m_Keys = (FloatArray*) NULL;
		return;
	}
	if (GetKeys())
		m_Keys->SetMaxSize(size * m_ValSize);
	else
		m_Keys = new FloatArray(size * m_ValSize);
}

int Interpolator::GetMaxSize() const
{
	if (GetKeys() == NULL)
		return 0;
	intptr n =  m_Keys->GetMaxSize() / m_ValSize;
	VX_ASSERT(n < INT_MAX);
	return (int) n;
}

void Interpolator::ClearDest()
{
	if (m_Dest == NULL)
		return;
	int vs = m_ValSize;
	if ((m_InterpType == SLERP) || (m_InterpType == QSTEP))
		++vs;
	memset(m_Dest, 0, vs * sizeof(float));
}

bool Interpolator::QuatBlend(const Quat *q)
{
	WQuat* dest = (WQuat*) m_Dest;
	WQuat* result = (WQuat*) GetResult();

	if (dest == NULL)
		return false;
	if (result)
	{
		*result = *q;
		result->weight = 1.0f;
	}
	if (m_Alpha == 1.0f)
	{
		*((Quat*) m_Dest) = *q;
		dest->weight = 1.0f;
	}
	else
	{
		Quat tmp;
		float wsum = m_Alpha + dest->weight;
		float blend_param;

		if (wsum == 0)
			return true;
		blend_param = m_Alpha / wsum;
		dest->Slerp(*dest, *q, blend_param);
		dest->weight = wsum;
	}
	return true;
}

bool Interpolator::LinearEval(const float *k1, const float *k2, float s)
{
	bool replace = (m_Alpha == 1.0f);
	float* result = (float*) GetResult();

	for (int i = 0; i < m_ValSize; i++)
	{
		float tmp = k1[i] + s * (k2[i] - k1[i]);
		if (result)
			result[i] = tmp;
		if (replace)
			m_Dest[i] = tmp;
		else
			m_Dest[i] += m_Alpha * tmp;
	}
	return true;
} 

bool Interpolator::SlerpEval(const float *k1, const float *k2, float s)
{
	bool replace = (m_Alpha == 1.0f);
	WQuat q;
	const WQuat* qk1 = (const WQuat*) k1;
	const WQuat* qk2 = (const WQuat*) k2;

	q.Slerp(*qk1, *qk2, s);
	return QuatBlend(&q);
} 
 


float Interpolator::GetTime(int i) const
{
	if (GetKeys() == NULL)
		return 0;
	intptr  n = m_Keys->GetSize();
	if (n == 0)
		return 0;
	if (i < 0)
		return 0;
	return (i * GetDuration() * m_ValSize) / n;
}

const float* Interpolator::GetKey(int i) const
{
	const float* data;

	if ((i < 0) || (GetKeys() == NULL))
		return NULL;
	i *= m_ValSize;
	VX_ASSERT(i < INT_MAX);
	if (i >= (int) m_Keys->GetSize())
		return NULL;
	data = m_Keys->GetData();
	data += i;
	return data;
}

/*!
 * @fn bool Interpolator::AddKey(float t, const float *key)
 * @param t		time stamp for key frame (in sec)
 *				negative time indicates put at the end
 * @param key	floating data for key (must have GetKeySize() elements)
 *
 * Adds a new key sample to the data area. Only the keys are stored.
 * The input time indicates  when this key should occur and
 * determines where the key will be put in relation to other keys.
 * The samples are assumed to be uniformly distributed in time
 * with a constant time increment between them. The index of the key
 * in the data area is derived From the time increment and the input time.
 *
 * @note If samples are not added in time order, gaps may occur in the
 * data area. Missing keys are initialized as zero.
 *
 * @return \b true if key frame successfully added, else \b false
 *
 * @see KeyFramer::AddKey VXKeySampler::SetKey KeyFramer::SetKeySize KeyFramer::SetControl
 */
bool Interpolator::AddKey(float t, const float *key)
{
	VX_STREAM_BEGIN(s)
	if (key)
	{
		*s << OP(VX_Interpolator, INTERP_AddKey) << this << t;
		s->Output(key, m_ValSize);
	}
	VX_STREAM_END( )

	FloatArray* keys = m_Keys;
	intptr 		index;

	if (keys == NULL)
		m_Keys = keys = new FloatArray(0);
	index = keys->GetSize();
	if ((index == 0) || (t < 0)	|| (t >= m_Duration))	// put at the end.
	{
		keys->SetSize(index + m_ValSize);
		if ((m_Duration < t) && (t > 0))
			m_Duration = t;
	}
	else												// replace existing key
		index = int(t / m_Duration);
	float*	data = keys->GetData() + index;
	memcpy(data, key, m_ValSize * sizeof(float));
	return true;
}

/*!
 * @fn void Interpolator::ScaleKeys(float v)
 * @param v	scale factor to apply
 *
 * Multiplies all the key values by the given scale factor.
 * The times are not affected. This is useful if the position tracks
 * are in a different coordinate system than the skeleton (meters
 * vs centimeters for example).
 *
 * @see KeyFramer::AddKey Scriptor::SetAnimationScale
 */
void Interpolator::ScaleKeys(float v)
{
	if ((v == 1.0f) || (v == 0.0f))
		return;
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Interpolator, INTERP_ScaleKeys) << this <<v;
	VX_STREAM_END( )

	int size = GetSize();
	for (int index = 0; index < size; ++index)
	{
		float* key = (float*) GetKey(index);
		for (int j = 0; j < GetValSize(); ++j)
			*key++ *= v;
	}
}

/*!
 * @fn bool Interpolator::SelectKeys(float& t1, float& t2, const float** k1, const float** k2)
 * @param t1	has evaluation time on input, gets time of first key on output
 * @param t2	gets time of second key
 * @param k1	gets pointer to data for first key
 * @param k2	gets pointer to data for second key
 *
 * This function selects the two keys to interpolate between for the current frame.
 * It is called each frame by Interpolator::ComputeValue.
 *
 * @return  true if time value between input keys, \b false for time out of ranege
 */
bool Interpolator::SelectKeys(float& t1, float& t2, const float** key1, const float** key2)
{
	if (m_Keys == NULL)
	{
		t1 = 0;
		t2 = GetDuration();
		*key1 = m_Dest;
		*key2 = NULL;
		return false;
	}
	
	int		size = GetSize();				// get number of keys
	float	t = t1;
	int		index;
	float*	data = m_Keys->GetData();

	if (size <= 0)							// no keys, nothing to do
		return false;
	if (t < 0)								// before first key
	{
		t2 = t1 = 0;
		*key2 = *key1 = data;
		return true;
	}
	if (m_Duration == 0)					// just have keys, no time info?
	{
		t1 = t2 = t;						// no interp, different key each frame
		*key2 = *key1 = data + m_LastKey * m_ValSize;
		m_LastKey++;
		return true;
	}
	index = (int) (t * size / m_Duration);	// get sample index
	if (index >= size)						// outside sample range?
		return false;
	VX_ASSERT(index < INT_MAX);
	t1 = GetTime(index);					// get first key
	*key1 = data + index * m_ValSize;
	m_LastKey = index;
	if (index < (size - 1))					// clamp at last sample
		index++;
	t2 = GetTime(index);					// get second key
	*key2 = data + index * m_ValSize;
	return true;
}

/*!
 * @fn const float* Interpolator::ComputeValue(float t)
 *
 * Evaluates the output value for the interpolator for the given time.
 * If the input time falls between two keys, the time is used to
 * interpolate a value between the two keys.
 * Depending on the interpolation type, spherical or linear
 * interpolation is performed. The selected key is multiplied by
 * the weight and added to the destination:
 * @code
 *	s = (t - t1) / (t2 - t1)
 *	result = s * key1 + (1 - s) * key2
 *	dest += alpha * result
 * @endcode
 *
 * @see Evaluator::SetAlpha Evaluator::SetDest Interpolator::SelectKeys
 */
const float* Interpolator::ComputeValue(float t)
{
	const float* k1;
	const float* k2;
	float t1 = t, t2 = t;

	if (!SelectKeys(t1, t2, &k1, &k2))
		return NULL;
	if (t1 == t2)
		return k1;
	Engine *par = (Engine*) Parent();
	if (par)
		par->SetChanged(true);
	float s = (t - t1) / (t2 - t1);
	switch (m_InterpType)
	{
		case SLERP:		SlerpEval(k1, k2, s); break;
		case LINEAR:	LinearEval(k1, k2, s); break;
		case QSTEP:		QuatBlend((Quat*) k1); break;		
		default:		BlendEval(k1); break;
	}
	return NULL;
}

const float* Interpolator::GetResult() const
{	return NULL; }

DebugOut& Evaluator::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Engine::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "\t<attr name='Alpha'>" << m_Alpha << "</attr>");
	endl(dbg << "\t<attr name='DestType'>" << m_DestType << "</attr>");
	endl(dbg << "\t<attr name='ValSize'>" << m_ValSize << "</attr>");
	return Engine::Print(dbg, opts & PRINT_Trailer);
}

DebugOut& Interpolator::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Evaluator::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "\t<attr name='InterpType'>" << m_InterpType << "</attr>");
	if (opts & PRINT_Data)
	{
		int numkeys = GetSize();
		endl(dbg << "\t<attr name='Keys'>");
		for (int i = 0; i < numkeys; ++i)
		{
			float t = GetTime(i);
			const float* key = GetKey(i);

			if (key == NULL)
				 break;
			dbg << "\t" << t;
			for (int k = 0; k < m_ValSize; ++k)
				dbg << " " << *key++;
			endl(dbg);
		}
		endl(dbg << "\n\t</attr>");
	}
	return Evaluator::Print(dbg, opts & PRINT_Trailer);
}

bool Vec3Blender::OnStart()
{
	GroupIter<Evaluator> iter(this, Group::CHILDREN);
	Evaluator*	e;
	int			n = 0;

	while (e = iter.Next())
		if (e-IsClass(VX_Evaluator) && (e->GetValSize() == 1))
			switch (e->GetDestType())
			{
				case XPOS:	// try to connect X
				if (m_XInput == NULL)
					e->SetDest(&m_Result.x);
				break;
	
				case YPOS:		// try to connect Y
				if (m_YInput == NULL)
					e->SetDest(&m_Result.y);
				break;

				case ZPOS:		// try to connect Z
				if (m_ZInput == NULL)
					e->SetDest(&m_Result.z);
				break;
			}
	return true;
}

const float* Vec3Blender::ComputeValue(float t)
{
	if (m_XInput)
		m_Result.x = *m_XInput;
	if (m_YInput)
		m_Result.y = *m_YInput;
	if (m_ZInput)
		m_Result.z = *m_ZInput;
	return (const float*) &m_Result;
}

void Vec3Blender::SetResult(const float* r)
	{ m_Result = *((const Vec3*) r); }

const float* Vec3Blender::GetResult() const
	{ return (const float*) &m_Result; }

}	// end Vixen