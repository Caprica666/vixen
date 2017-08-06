#include "vixen.h"
#include "vxutil.h"

namespace Vixen {

/*!
 * @fn int QuadTree::CullData(Box2 bound, QuadRoot *root)
 * @param bound	bounds to cull within
 * @param root	root of quad tree
 *
 * Compares each model in the local set and culls those which
 * are not within the input bounds
 *
 * @return number of models kept
 */
int QuadTree::CullData(Box2 bound, QuadRoot *root)
{
	for (int i = 0; i < m_Prims.GetSize(); i++)
	{
		if (!root->BoundCheck(bound, m_Prims[i]))
		{
			m_Prims.RemoveAt(i);
			--i;
		}
	}
	return (int) m_Prims.GetSize();
}

QuadRoot::QuadRoot(int thresh, int limit) : SharedObj()
{
	SetThreshold(thresh);
	SetLimit(limit);
}


/*!
 * @fn Vec2 QuadRoot::Planify(const Vec3 &vert)
 *
 * Projects the input vertex onto the ground plane.
 * Currently, the ground plane is hard-coded as the XZ plane
 * so the routine returns Vec2(vert.x, vert.z)
 */
Vec2 QuadRoot::Planify(const Vec3 &vert)
{
	return Vec2(vert.x, vert.z);
}

/*!
 * @fn QuadTree* QuadRoot::Descend(const Vec3 &location)
 * @param location	location to check
 *
 * Find the node which contains a list of possible hits for
 * the input point
 */
const QuadTree* QuadRoot::Descend(const Vec3 &location) const
{
	Vec2 loc = Planify(location);

	if (m_pRoot == (const QuadTree*) NULL)		// if there is currently no tree, bail out
		return NULL;
    
	Box2 loc_bound = m_Bound;				//  if original location is out of bounds, bail
	if ((loc.x < loc_bound.min.x) || (loc.x >= loc_bound.max.x) || 
		(loc.y < loc_bound.min.y) || (loc.y >= loc_bound.max.y))
		return NULL; 

	const QuadTree* pCurrent = m_pRoot;

	while (pCurrent && pCurrent->NodeType() == QN_INTERNAL)
	{
		Vec2 center = loc_bound.Center();
		if (loc.x < center.x)
		{
			if (loc.y < center.y)
			{
				//quad 0
				loc_bound.max = center;
				pCurrent = pCurrent->m_apNode[0];
			}
			else
			{
				//quad 2
				loc_bound.min.y = center.y;
				loc_bound.max.x = center.x;
				pCurrent = pCurrent->m_apNode[2];
			}
		}
		else
		{
			if (loc.y < center.y)
			{
				//quad 1
				loc_bound.min.x = center.x;
				loc_bound.max.y = center.y;
				pCurrent = pCurrent->m_apNode[1];
			}
			else
			{
				//quad 3
				loc_bound.min = center; 
				pCurrent = pCurrent->m_apNode[3];
			}
		}
	}
	return pCurrent;
}

bool QuadRoot::BuildTree()
{
	int i;

	VX_ASSERT(m_Size >0);
	if (m_Size <= 0)					// make sure size has been properly set
		return false;
	m_pRoot = new QuadTree(QN_CONTAINER);
	VX_ASSERT(m_pRoot != NULL);
	if (m_pRoot == (const QuadTree*) NULL)
		return false;
	for (i = 0; i < m_Size; i++)		// create root node initialized to contain every prim.
		m_pRoot->m_Prims.Append(i);
	m_pRoot = BuildRecurse(m_pRoot, m_Bound, m_Threshold, 0, m_Limit);
	if (m_pRoot == (const QuadTree*) NULL)	// expand tree from seed node using recursion
		return false;
	DebugTreeData(&m_debugData);
	return true;
}

QuadTree* QuadRoot::BuildRecurse(QuadTree *node, const Box2& bound, int thresh, int depth, int limit)
{
	int tmp;
	if ((tmp = node->CullData(bound, this)) == 0)	// base case 1: node is empty, return zero node
	{
//		node->Delete();
		return NULL;
	}
	if (tmp < thresh)								// base case 2: threshold reached, node does not need to be divided
		return node;
	if (depth >= limit)								// base case 3: depth limit reached, do not divided any further
		return node;

	QuadTree* new_node = new QuadTree();		// make new internal node
	for (int i = 0; i < 4; i++)
	{
		QuadTree* child_node = new QuadTree(QN_CONTAINER);
		VX_ASSERT(child_node != NULL);			// create new child container node
		child_node->SetData(&(node->m_Prims));	// give it the parent node's data
		Box2 subBound = QuadTree::SubBound(bound, i);	// get bound for new child node
		new_node->m_apNode[i] = BuildRecurse(child_node, subBound, thresh, depth + 1, limit);
	}
//	node->Delete();								// delete old container and return new internal node
	return new_node;
}

const IntArray* QuadRoot::GetHitList(Vec3 testPoint) const
{
	const QuadTree* node = Descend(testPoint);
	if (node == NULL)
		return NULL;
	if (node->m_Prims.GetSize() == 0)
		return NULL;
	return &(node->m_Prims);
}

Box2 QuadTree::SubBound(const Box2 &bound, int which)
{
	Box2 new_bound;
	Vec2 center = bound.Center();

	switch (which)
	{
		case 0:
		new_bound.min = bound.min;
		new_bound.max = center;
		break;

		case 1:
		new_bound.min.x = center.x;
		new_bound.min.y = bound.min.y;
		new_bound.max.x = bound.max.x;
		new_bound.max.y = center.y;
		break;

		case 2:
		new_bound.min.x = bound.min.x;
		new_bound.min.y = center.y;
		new_bound.max.x = center.x;
		new_bound.max.y = bound.max.y;
		break;

		case 3:
		new_bound.min = center;
		new_bound.max = bound.max;
		break;

		default:
		VX_ASSERT(0);
	}
	return new_bound;
}

void QuadRoot::RecurseTreeData(QuadTree* node, TreeData *data)
{
	if (node == NULL)
	{
		data->numZero++;
        data->sumContDepth += data->depth;
        data->maxDepth = (data->depth > data->maxDepth) ? data->depth : data->maxDepth;
		return;
	}

	int type = node->NodeType();
    switch (type)
    {
		case QN_INTERNAL:
        data->numInternal++;
        data->depth++;

        RecurseTreeData(node->m_apNode[0], data);
        RecurseTreeData(node->m_apNode[1], data);
        RecurseTreeData(node->m_apNode[2], data);
        RecurseTreeData(node->m_apNode[3], data);
        data->depth--;
        break;

		case QN_CONTAINER:
        data->numContainer++;
        data->sumListSize += (int) node->m_Prims.GetSize();
        data->sumContDepth += data->depth;
        data->maxDepth = (data->depth > data->maxDepth) ? data->depth : data->maxDepth;
        break;

		default:
        VX_ASSERT(0);
        break;
    }
    return;
}

void QuadRoot::DebugTreeData(TreeData* data)
{
    if (m_pRoot != NULL)
        RecurseTreeData(m_pRoot, data);
    if (data->numContainer != 0)
    {
        data->aveContDepth = data->sumContDepth / (float) data->numContainer;
        data->aveListSize  = data->sumListSize / (float) data->numContainer;
    }
}
}	// end Vixen