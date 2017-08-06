#include "vixen.h"

namespace Vixen {

/****
 *
 * class KeyFramer
 *
 ****/
VX_IMPLEMENT_CLASSID(KeyFramer, Interpolator, VX_KeyFramer);

static const TCHAR* opnames[] =
{
	TEXT("SetMaxSize"),
	TEXT("AddKey"),
	TEXT("SetSize"),
	TEXT("SetPart"),
};

const TCHAR** KeyFramer::DoNames = opnames;


/*!
 * @fn int KeyFramer::Find(const float* val, float dist)
 * @param val	value to search for. If NULL, the current value of
 *				the keyframer's destination is used.
 * @param dist	minimum acceptable distance. If zero, the keyframer	
 *			is sampled at all key frames to find closest sample.
 *
 * Finds the index of the key that is closest to the input value.
 * The keyframer is evaluated over its range to obtain samples.
 * The closest sample is the one whose vector distance from
 * the input value is the smallest.
 *
 * If a non-zero input distance is given, sampling is stopped
 * when a sample within that distance is found (even if it is
 * not the closest one). If none of the samples are within the
 * specified distance, -1 is returned. A zero distance will sample
 * the keyframer at all key frames and return the index of the closest.
 *
 * @return index of key with closest value or -1 if none found
 *
 * @see KeyFramer::GetAt KeyFramer::GetKey
 */
int KeyFramer::Find(const float* val, float dist)
{
	int32	n = GetSize();
	float	mindist = FLT_MAX;
	int32	minidx = -1;
	float	d = 0;

	if (n <= 0)
		return -1;
	dist *= dist;					// work with distance squared
	for (int32 i = 0; i < n; i++)	// scan keys to find closest one
	{
		const float* k = GetKey(i);
		d = 0;
		for (int j = 0; j < m_ValSize;	++j)
			d += (val[j] - k[j]) * (val[j] - k[j]);	
		if (d < mindist)			// closest one so far?
		{
			minidx = i;				// save its index
			if (fabs(mindist - d) < FLT_EPSILON)
				break;
			mindist = d;
		}
		else if (d < dist)			// meets input criteria for closest?
		{
			minidx = i;
			mindist = d;
			break;					// return this one	
		}			
	}
	if ((dist > 0) && (d > dist))	// none within input distance?
		return -1;				
	return minidx;					// return index of closest
}

/****
 *
 * Name: KeyFramer::SelectKeys
 *
 * Description:
 *	Internal routine to allow an interpolator to return the two keys to
 *	interpolate. The behavior for a KeyFramer is to return the two keys
 *	closest to the input time.
 *
 ****/
bool KeyFramer::SelectKeys(float& t1, float& t2, const float** key1, const float** key2)
{
	int32	size = GetSize();
	float	t = t1;
#if _TRACE > 1
	int		debug = (Engine::Debug > 1) || KeyFramer::Debug;
#endif

	if (size <= 0)
		return false;
/*
 * Normal keyframer maintains all keys and allows time-addressibility
 * both backwards and forwards
 */
	t1 = GetTime(m_LastKey);
	while (t < t1 && m_LastKey > 0)
		t1 = GetTime(--m_LastKey);
	if (t < t1)							// before beginning
	{
		t1 = t2 = GetTime(0);
		*key1 = *key2 = GetKey(0);
		VX_TRACE2(debug, ("KeyFramer::Eval: %s %f first\n", GetName(), t2));
		return true;
	}
	--size;
	if (m_LastKey >= size)				// at the end?
	{
		t1 = t2 = GetTime(size);
		if (t1 < t)
			return false;
		*key1 = *key2 = GetKey(size);
		VX_TRACE2(debug, ("KeyFramer::Eval: %s %f last\n", GetName(), t2));
		return true;
	}
	t2 = GetTime(m_LastKey + 1);
	while (t > t2 && m_LastKey < size)	// search for another key
	{
		t1 = t2;
		t2 = GetTime(++m_LastKey + 1);
	}
	if (m_LastKey >= size)
	{
		t1 = t2 = GetTime(size);
		*key1 = *key2 = GetKey(size);
	}
	else
	{
		*key1 = GetKey(m_LastKey);		// return two interpolation keys
		*key2 = GetKey(m_LastKey + 1);
	}
	VX_TRACE2(debug, ("KeyFramer::Eval: %s %f -> %f\n", GetName(), t1, t2));
	return true;
}

KeyFramer::KeyFramer() : Interpolator()
{
	m_Part = 0;
}

bool KeyFramer::OnStart()
{
	m_LastKey = 0;
	return true;
}

bool KeyFramer::Copy(const SharedObj* srcobj)
{
	ObjectLock dlock(this);
	ObjectLock slock(srcobj);
	if (!Interpolator::Copy(srcobj))
		return false;

	if (srcobj->IsClass(VX_KeyFramer))
	{
		const KeyFramer* src = (const KeyFramer*) srcobj;
		m_LastKey = src->m_LastKey;
		m_Part = src->m_Part;
	}
	return true;
}


bool KeyFramer::Do(Messenger& s, int op)
{
	float	v;
	int32	n;
	float	key[MAX_KEY_SIZE];

	switch (op)
	{
		case KEY_SetMaxSize:
		s >> n;
		SetMaxSize(n);
		break;

		case KEY_SetSize:
		s >> n;
		SetSize(n);
		break;

		case KEY_AddKey:
		s >> n >> v;
		VX_ASSERT(n == m_ValSize);
		s.Input(key, n);
		AddKey(v, key); 
		break;

		case KEY_SetPart:
		s >> n;
		SetPart(n);
		break;

		default:
		return Interpolator::Do(s,op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << KeyFramer::DoNames[op - KEY_SetMaxSize]
					   << " " << this);
#endif
	return true;
}


int KeyFramer::Save(Messenger& s, int opts) const
{
	int32 h = Interpolator::Save(s, opts);
	if (h <= 0)
		return h;
	if (m_Part)
		s << OP(VX_KeyFramer, KEY_SetPart) << h << int32(m_Part);
	return h;
}

void KeyFramer::SetSize(int size)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_KeyFramer, INTERP_SetSize) << this << int32(size);
	VX_STREAM_END( )

	if (GetKeys() == NULL)
		m_Keys = new FloatArray(size);
	else
		m_Keys->SetSize(size);
}

void KeyFramer::SetMaxSize(int size)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_KeyFramer, INTERP_SetMaxSize) << this << int32(size);
	VX_STREAM_END( )

	if (GetKeys() == NULL)
		m_Keys = new FloatArray(size);
	else
		m_Keys->SetMaxSize(size);
}

float KeyFramer::GetTime(int i) const
{
	if ((i < 0) || (GetKeys() == NULL))
		return 0;
	i *= m_ValSize;
	VX_ASSERT(i < INT_MAX);
	if (i >= (int) m_Keys->GetSize())
		return 0.0f;
	return m_Keys->GetAt(i);
}

const float* KeyFramer::GetKey(int i) const
{
	if ((i < 0) || (GetKeys() == NULL))
		return NULL;
	i *= m_ValSize;
	VX_ASSERT(i < INT_MAX);
	if (i >= (int) m_Keys->GetSize())
		return NULL;
	return (const float*) &m_Keys->GetAt(i) + 1;
}

int KeyFramer::GetPart() const
	{ return m_Part; }


/*!
 * @fn bool KeyFramer::AddKey(float t, const float *key)
 * @param t		time stamp for key frame (in sec), may be interpreted
 * @param key	floating data for key (must have GetValSize() elements)
 *
 * Adds a new key frame to the data area.
 * A key frame includes both a time and a key value.
 * The key frames are kept in time order.
 * Normal keyframers will grow dynamically to accomodate more keys.
 *
 * @return \b true if key frame successfully added, else \b false
 *
 * @see Evaluator::SetValSize Interpolator::GetKey
 */
bool KeyFramer::AddKey(float t, const float *key)
{
	VX_STREAM_BEGIN(s)
	if (key)
	{
		*s << OP(VX_KeyFramer, INTERP_AddKey) << this << t;
		s->Output(key, m_ValSize);
	}
	VX_STREAM_END( )

	FloatArray* keys = m_Keys;
	if (keys == NULL)
		m_Keys = keys = new FloatArray(0);
	// Add key at the end if there are no keys or its time is larger
	// than the last key
	int		i, index;
	int		n = (int) keys->GetSize();
	float*	keydata = (float*) &keys->GetAt(n);
	bool	append = true;

	VX_ASSERT(n < INT_MAX);
	// Key must be put between other keys or at the beginning
	// because its time is smaller the one of the other keys
	if ((n > 0) && (t < *(keys->Last())))
		for (index = 0; index < n; index += m_ValSize)
		{
			float keytime = *keydata;
			if (keytime == t)				// time same as existing key?
			{
				n = index;					// dont shuffle, just replace the key
				append = false;
				break;
			}
			else if (keytime > t)
			{
				append = false;
				break;
			}
		}
	// Key must be put at the end of the array
	if (append)
	{
		keys->SetSize(n + m_ValSize);	// keydata will change if array grows
		keydata = (float*) &keys->GetAt(n);
		*keydata = t;
		for (i = 0; i < m_ValSize; i++)
			keydata[i + 1] = key[i];
		return true;
	}

	// Shuffle the keys after the index down one to make room for 
	// the new key
	for (i = n; i >= index; ++i)
		keys->SetAt(i + m_ValSize, keys->GetAt(i));

	// Add the new time and key
	keydata = (float*) &keys->GetAt(index);
	keydata[0] = t;
	for (i = 0; i < m_ValSize; i++)
		keydata[i + m_ValSize] = key[i];
	return true;
}

void KeyFramer::SetPart(int part)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_KeyFramer, KEY_SetPart) << this << int32(part);
	VX_STREAM_END(  )

	m_Part = part;
}

/****
 *
 * class KeyBlender
 *
 ****/
VX_IMPLEMENT_CLASSID(KeyBlender, Evaluator, VX_KeyBlender);

KeyBlender::KeyBlender() : Evaluator()
{
	SetControl(CHILDREN_FIRST | CONTROL_CHILDREN);
	SetValSize(0);
	memset(m_Result, 0, m_ValSize);
}

void KeyBlender::SetResult(const float* v)
{
	VX_ASSERT(m_ValSize < MAX_KEY_SIZE);
	memcpy(m_Result, v, m_ValSize);
}

const float* KeyBlender::GetResult() const
{
	return m_Result;
}

void KeyBlender::ClearDest()
{
	if (m_Dest == NULL)
		return;
	int vs = m_ValSize;
	if (GetDestType() == ROTATION)
		++vs;
	memset(m_Dest, 0, vs * sizeof(float));
}

bool KeyBlender::Eval(float t)
{
	const float* v = ComputeValue(t);

	if (v == NULL)
		return true;
	if (GetDest() == NULL)
		return true;
	Engine *par = (Engine*) Parent();
	if (par)
		par->SetChanged(true);
	if (GetDestType() == ROTATION)
	{
		Quat*	dest = (Quat*) GetDest();

		dest->Slerp(*dest, *((Quat*) v), m_Alpha);
	}
	else
	{
		for (int i = 0; i < m_ValSize; ++i)
			m_Dest[i] = (1 - m_Alpha) * m_Dest[i] + m_Alpha * v[i];
	}
	return true;
}

const float* KeyBlender::ComputeValue(float t)
{
	return GetResult();
}

float	KeyBlender::ComputeTime(float t)
{
	t = Evaluator::ComputeTime(t);
	if (t < 0.0f)
		return t;
	if (!Setup(t))
		return -1.0f;
	return t;
}

bool KeyBlender::Setup(float t)
{
	int			c = GetControl();
	Evaluator*	firstInterp = FindInterp(NULL);
	float		alpha = t / m_Duration;
	float		empty[4] = { 0, 0, 0, 1 };

	if (firstInterp == NULL)		// no child interpolators?
		return false;
	if (m_Duration == 0)
		alpha = 1.0f;
	if ((c & (BLEND_TO | BLEND_BETWEEN)) == 0)
		alpha = 1.0f;
/*
 * Blend between destination and start of child keyframer
 */
	if (c & BLEND_TO)
	{
		SetAlpha(alpha);
		firstInterp->SetAlpha(1.0f);
		return true;
	}
/*
 * Blend between two keyframers
 */
	Evaluator*	secondInterp = NULL;

	secondInterp = FindInterp(firstInterp);
	SetAlpha(1.0f);
	if ((secondInterp == NULL) || ((c & BLEND_BETWEEN) == 0))
	{
		firstInterp->SetAlpha(1.0f);
		return true;
	}
	if (m_Duration == 0)
		alpha = 1.0f;
	VX_ASSERT((alpha >= 0) && (alpha <= 1));
	ClearDest();
	firstInterp->ClearDest();
	if (firstInterp->GetStartTime() <= secondInterp->GetStartTime())
	{
		firstInterp->SetAlpha(1 - alpha);
		secondInterp->SetAlpha(alpha);
	}
	else
	{
		secondInterp->SetAlpha(1 - alpha);
		firstInterp->SetAlpha(alpha);
	}
	return true;
}

Evaluator* KeyBlender::FindInterp(Engine* g)
{
	Evaluator* first = (Evaluator*) g;

	if (first == NULL)
		g = First();
	else if (g)
		g = g->Next();
	while (g)
	{
		if (g->IsClass(VX_Interpolator) && g->IsRunning())
			return (Evaluator*) g;
		g = (Engine*) g->Next();
	}
	return NULL;
}

bool KeyBlender::OnStart()
{
	GroupIter<Engine> iter(this, Group::CHILDREN);
	Engine*	e;

	while (e = iter.Next())
	{
		if (e->IsClass(VX_Interpolator))
		{
			Evaluator* intrp = (Evaluator*) e;
			intrp->SetDest(m_Result);
		}
	}
	return true;
}

bool KeyBlender::OnStop()
{
	if (GetDuration())				// if keyframer children
	{
		SetDuration(0, ONLYME);		// don't stop
		VX_TRACE(Animator::Debug, ("KeyBlender::OnStop %s\n", GetName()));
		return false;
	}
	return true;
}

VX_IMPLEMENT_CLASSID(ColorInterp, KeyFramer, VX_ColorInterp);

bool ColorInterp::Eval(float t)
{
	SharedObj*	target = GetTarget();
	const Col4&	col = m_ColorResult;
	
	Key<Col4>::Eval(t);
	if (target == NULL)
	{
		Scene* scene = GetMainScene();

		switch (m_Part)
		{
			case Scene::SCENE_SetAmbient:
			scene->SetAmbient(col);
			return true;

			case Scene::SCENE_SetBackColor:
			scene->SetBackColor(col);
			return true;
		}
		return true;
	}
	if (target->IsClass(VX_Light))
	{
		((Light*) target)->SetColor(col);
		return true;
	}
	if (target->IsClass(VX_Material))
	{
		switch (GetPart())
		{
			case PhongMaterial::MAT_SetSpecular:
			((PhongMaterial*) target)->SetSpecular(col);
			break;

			case PhongMaterial::MAT_SetAmbient:
			((PhongMaterial*) target)->SetAmbient(col);
			break;

			case PhongMaterial::MAT_SetEmission:
			((PhongMaterial*) target)->SetEmission(col);
			break;

			default:
			((PhongMaterial*) target)->SetDiffuse(col);
			break;
		}
		return true;
	}
	return true;
}


VX_IMPLEMENT_CLASSID(TimeInterp, KeyFramer, VX_TimeInterp);


TimeInterp::TimeInterp() : Key<float>()
{
	SetDest(&m_TimeResult);
	m_TimeResult = 0.0f;
	SetControl(CONTROL_CHILDREN);
}

void TimeInterp::ComputeChildren(float t, int opts)
{
	GroupIterNotSafe<Engine> iter(this, CHILDREN);
	Engine*	g;
	float	gt;

	while (g = iter.Next())
	{
		float dur = g->GetDuration();
		if (dur == 0)
			dur = m_TimeScale;
		gt = m_TimeResult * dur + g->GetStartTime();
		VX_TRACE((Engine::Debug > 1) || TimeInterp::Debug, ("TimeInterp::Eval: %s %f -> %f\n",
				GetName(), m_TimeResult, gt));
		g->Compute(gt);
	}
}

bool TimeInterp::OnStart()
{
	GroupIterNotSafe<Engine> iter(this, Group::DEPTH_FIRST);
	Engine*	e;

	if (m_TimeScale > 0)
		return true;
	while (e = iter.Next())
	{
		if (e == this)
			continue;
		e->SetTimeOfs(0, ONLYME);
		if (e->GetDuration() > m_TimeScale)
			m_TimeScale = e->GetDuration();
	}	
	return true;
}


}	// end Vixen