#pragma once

namespace Vixen {
#define QN_CONTAINER 1
#define QN_INTERNAL  2

typedef bool (*BoundCheckFunc)(Box2, int);

class QuadRoot;

/*!
 * @class QuadTree
 * @brief child node in a quad tree used to sort triangles spatially for collision detection.
 *
 * @see QuadRoot QuadTerrain
 * @internal
 */
class QuadTree : public SharedObj
{
public:
	friend class QuadRoot;
	QuadTree(int nodetype = QN_INTERNAL) : SharedObj() { m_NodeType = nodetype; }
	~QuadTree() {}

	//! Return type of node (container or internal)
	int		NodeType() const			{ return m_NodeType; }

	//! Set array of primitives to hit test against
	void	SetData(IntArray* prims)	{ m_Prims.Copy(prims); }

	const IntArray* GetData() const	{ return &m_Prims; }

	//! eliminate primitives outside given bound
	int		CullData(Box2 bound, QuadRoot *Q);  

	static	Box2 SubBound(const Box2 &bound, int which);

protected:
	int					m_NodeType;
	Ref<QuadTree>		m_apNode[4];
	IntArray			m_Prims;
};

/*!
 * @class QuadRoot
 * @brief root node of a quad tree used to sort triangles spatially for collision detection.
 *
 * @see QuadTree QuadTerrain
 * @internal
 */
class QuadRoot : public SharedObj
{
	struct TreeData
	{
		int32 depth;
		int32 numInternal;
		int32 numContainer;
		int32 numZero;
		int32 sumContDepth;
		int32 sumListSize;
		int32 maxDepth;
		float aveContDepth;
		float aveListSize;

		TreeData()
		{
			depth = 0;
			numInternal = 0;
			numContainer = 0;
			numZero = 0;
			sumContDepth = 0;
			sumListSize = 0;
			maxDepth = 0;
			aveContDepth = 0.0f;
			aveListSize = 0.0f;
		}

	};

public:
	QuadRoot(int thresh = 5, int limit = 6);

	void SetBound(float left, float right, float top, float bottom)
	{ m_Bound.Set(left, right, top, bottom); }

	void SetBound(Box3 bound) 
	{
		Vec2 min = Planify(bound.min);
		Vec2 max = Planify(bound.max);
		m_Bound.Set(min.x, max.x, min.y, max.y);
	}    

	virtual bool BoundCheck(const Box2 &bound, int primindex) = 0;

	const IntArray* GetHitList(Vec3 testPoint) const;

	void SetThreshold(int thresh)	{ m_Threshold = thresh; }
	void SetLimit(int limit)		{ m_Limit = limit; }
	void SetNumPrims(int size)		{ m_Size = size; }

	//--- Call this when all of the "Set" functions have been called ---
	bool BuildTree();

	//! Get the mode which contains possible hits
	const QuadTree*	Descend(const Vec3 &location) const;

	static Vec2 Planify(const Vec3 &vert);

	bool Valid()					{ return m_Valid; }

	void DebugTreeData(TreeData* data);
	void RecurseTreeData(QuadTree *node, TreeData *data);


	//! Recursive tree building function used by BuildTree
	QuadTree*	BuildRecurse(QuadTree* node, const Box2& bound, int thresh, int depth, int limit);

protected:
	Ref<QuadTree>	m_pRoot;
	Box2			m_Bound;
	int32			m_Size;
	int				m_Threshold;
	int				m_Limit;
	bool			m_Valid;
	TreeData		m_debugData;
};

} // end Vixen