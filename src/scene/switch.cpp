
#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(ModelSwitch, Model, VX_ModelSwitch);

static const TCHAR* opnames[] =
{	TEXT("SetIndex") };

const TCHAR** ModelSwitch::DoNames = opnames;

/****
 *
 * class ModelSwitch override for Model::Select
 * Selects the child to render based on the index.
 * If the index is out of range, the last child is displayed.
 *
 ****/
int ModelSwitch::Cull(const Matrix* trans, Scene* scene)
{
	if (Model::Cull(trans, scene) == DISPLAY_NONE)
		return DISPLAY_NONE;
	if (m_Index < 0)
		return DISPLAY_NONE;
	return DISPLAY_ME;
}

void ModelSwitch::Render(Scene* scene)
{
	Model* mod = GetAt(m_Index);
	if (mod)
		mod->Display(scene);
}

/****
 *
 * class ModelSwitch override for Model::CalcBound
 * Calculates the bounding volume of the currently
 * switched in child
 *
 ****/
bool ModelSwitch::CalcBound(Box3* b) const
{
	if (m_Index < 0)
		return false;
	const Model* mod = GetAt(m_Index);
	if (mod)
		return mod->CalcBound(b);
	return false;
}

/*!
 * @fn void ModelSwitch::SetIndex(int index)
 * @param index 0-based index of child to display
 *
 * If the Model::DYNAMIC hint is set for the switch node,
 * the bounding volume is recomputed when the index is updated (default case)
 *
 * @see Model::Cull Model::SetHints
 */
void ModelSwitch::SetIndex(int index)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_ModelSwitch, SWITCH_SetIndex) << this << int32(index);
	VX_STREAM_END(  )

	if (index != m_Index)
	{
		Core::InterlockSet(&m_Index, index);
		if (GetHints() & DYNAMIC)
			NotifyParents(MOD_BVinvalid);
	}
}
	
bool ModelSwitch::Copy(const SharedObj* src_obj)
{
	const ModelSwitch* src = (const ModelSwitch*) src_obj;

	if (!Model::Copy(src_obj))
		return false;
	if (src_obj->IsClass(VX_ModelSwitch))
		Core::InterlockSet(&m_Index, src->m_Index);
	return true;
}

/****
 *
 * class ModelSwitch override for SharedObj::Do
 *		SWITCH_SetIndex	<int32>
 *
 ****/
bool ModelSwitch::Do(Messenger& s, int op)
{
	int32		v;

	switch (op)
	   {
		case SWITCH_SetIndex:
		s >> v;
		SetIndex(v);
		break;

		default:
		return Model::Do(s, op);
	   }
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << ModelSwitch::DoNames[op - SWITCH_SetIndex]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * class ModelSwitch override for SharedObj::Save
 *
 ****/
int ModelSwitch::Save(Messenger& s, int opts) const
{
	int32 h = Model::Save(s, opts);
	if (h <= 0)
		return h;
	s << OP(VX_ModelSwitch, SWITCH_SetIndex) << h << (int32) GetIndex();
	return h;
}

}	// end Vixen