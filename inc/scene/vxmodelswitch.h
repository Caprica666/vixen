#pragma once

namespace Vixen {

/*!
 * @class ModelSwitch
 *
 * @brief Selects one of its children for display
 * based on an index supplied by the user.
 *
 * The switch model	does not have any geometry of its own but it doe
 * have a matrix. If the index is out of range, nothing is
 * displayed. Otherwise, the indexed child is displayed.
 * A switch will only ever display one of its children per frame.
 *
 * @see Model
 */
class ModelSwitch : public Model
{
public:
	VX_DECLARE_CLASS(ModelSwitch);

	ModelSwitch();

	void	SetIndex(int);		//!< Set switch index to select child to display.
	int		GetIndex() const;	//!< Get current switch index (zero based).

	virtual bool	Copy(const SharedObj*);
	virtual bool	Do(Messenger& s, int op);
	virtual int		Save(Messenger&, int) const;

	/*
	 * Switch::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		SWITCH_SetIndex = Model::MOD_NextOp,
		SWITCH_NextOp = Model::MOD_NextOp + 10,
	};

protected:
	virtual int		Cull(const Matrix*, Scene* scene);
	virtual void	Render(Scene*);
	virtual	bool	CalcBound(Box3*) const;

	vint32	m_Index;
};

inline int ModelSwitch::GetIndex() const
{	return m_Index; }

inline ModelSwitch::ModelSwitch() : Model()
{
	m_Index = 0;
}

} // end Vixen