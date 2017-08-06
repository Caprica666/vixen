#include "vixen.h"
#include "vxutil.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(TerrainCollider, Engine, VX_TerrainCollider);

static const TCHAR* opnames[] =
{
	TEXT("SetHeight"),
	TEXT("BuildTerrain"),
	TEXT("SetTerrain")
};

const TCHAR** TerrainCollider::DoNames = opnames;


TerrainCollider::TerrainCollider() : Engine()
{
	m_height = 0.0f;
	m_start.Set(FLT_MAX, FLT_MAX, FLT_MAX);
}

QuadTerrain* TerrainCollider::BuildTerrain(const Shape *model, int thresh, int max_depth)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_TerrainCollider, TERRCOLL_BuildTerrain) << this << model << int32(thresh) << int32(max_depth);
   VX_STREAM_END( )

	m_TerrainObject = new QuadTerrain(model, thresh, max_depth);
	if (!m_TerrainObject->IsValid())
		m_TerrainObject = (QuadTerrain*) NULL;
	return m_TerrainObject;
}

#define NUM_STEPS 6
#define ANG_INC PI/2/NUM_STEPS
#define NON_INITIALIZED 1.234E+20f

bool TerrainCollider::Eval(float t)
{
	Vec3 tmp_vec, end_pos;
	Quat tmp_q, tmp_r;
	Vec3 end, normal;  // normal is a dummy arg, not used
	Vec3 p, n;		   // Pos, Neg

	if ( ! m_evalReady) 
		return false;
	if (m_TerrainObject.IsNull())
		return false;

	Engine* par = Parent();
	if ( ! par) return true;
	if ( ! par->IsClass(VX_Transformer)) return true;



	//--- Get the start point and end point in world space ---
	((Transformer*)par)->CalcMatrix();
	end = ((Transformer*)par)->GetPosition();
	if (m_start.x == FLT_MAX)
		m_start = end;

	//--- test the navigation segment as given ---
	if (m_TerrainObject->HitInfo(end,normal))
	{
		end.y += m_height;
		((Transformer*)par)->SetPosition(end);
		m_start = end; // store for next frame
		return true;
	}
	
	//--- first test failed, we will now hunt left and right ---
	for (int i = 1; i <= NUM_STEPS; i++)
	{
		Matrix	trans;
		float	angle = i * ANG_INC;
		float	scale = float(cos(angle)+1)/2;
		Vec3	end_neg = end_pos = (end - m_start) * scale;
		bool	result_pos, result_neg;

		trans.RotationMatrix(Model::YAXIS,angle);
		trans.TransformVector(end_pos,tmp_vec);
		end_pos = tmp_vec + m_start;

		trans.RotationMatrix(Model::YAXIS,-angle); // opposite angle
		trans.TransformVector(end_neg,tmp_vec);
		end_neg = tmp_vec + m_start;
		result_pos = m_TerrainObject->HitInfo(end_pos,p);
		result_neg = m_TerrainObject->HitInfo(end_neg,n);

		if (!(result_pos || result_neg))
			continue;
		else if (result_pos && result_neg)
			break;
		else if (result_pos)
		{
			end = end_pos;
			end.y += m_height;
			((Transformer*) par)->SetPosition(end);
			m_start = end; // store for next frame
			return true;
		}
		else // result_neg
		{
			end = end_neg;
			end.y += m_height;
			((Transformer*) par)->SetPosition(end);
			m_start = end; // store for next frame
			return true;
		}
	}

	//--- we have failed to find a good move ---
	((Transformer*)par)->SetPosition(m_start);
	return true;
}



bool TerrainCollider::OnStart()
{
	if ( ! Engine::OnStart())
		return false;
	//--- returns false if terrain not valid ---
	if ( m_TerrainObject.IsNull() ||  (! m_TerrainObject->IsValid()))
		return false;
	m_evalReady = true;

	return true;
}

bool TerrainCollider::Copy(const SharedObj* src_obj)
{
	const TerrainCollider* src = (const TerrainCollider*) src_obj;
	
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Engine::Copy(src_obj))
		return false;
	if (src_obj->IsClass(VX_TerrainCollider))
	   {
		m_TerrainObject = src->m_TerrainObject;
		Lock();
		m_normal = src->m_normal;
		m_height = src->m_height;
		Unlock();
	   }
	return true;
}

void TerrainCollider::SetHeight(float h)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_TerrainCollider, TERRCOLL_SetHeight) << this << h;
   VX_STREAM_END( )

	m_height = h;
}

void TerrainCollider::SetTerrain(QuadTerrain *terrain)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_TerrainCollider, TERRCOLL_SetTerrain) << this << terrain;
   VX_STREAM_END( )

	m_TerrainObject = terrain;
}

bool TerrainCollider::Do(Messenger& s, int op)
{
	int32		thresh, depth;
	SharedObj*		obj;
	float		f;

	switch (op)
	{
		case TERRCOLL_SetHeight:
		s >> f;
		SetHeight(f);
		break;

		case TERRCOLL_SetTerrain:
		s >> obj;
		SetTerrain((QuadTerrain*) obj);
		break;

		case TERRCOLL_BuildTerrain:
		s >> obj >> thresh >> depth;
		VX_ASSERT(obj->IsClass(VX_Model));
		BuildTerrain((const Shape*) obj, thresh, depth);
		break;

		default:
		return Engine::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << TerrainCollider::DoNames[op - TERRCOLL_SetHeight]
					   << " " << this);
#endif
	return true;
}

}	// end Vixen