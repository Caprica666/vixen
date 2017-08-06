
#pragma once

namespace Vixen {
	/*!
	 * @class LevelOfDetail
	 * @brief A LevelOfDetail model selects one of its children to
	 * display based on the distance of the viewer from the
	 * level of detail object.
	 *
	 * You supply an array of ranges that
	 * indicates when each child should be displayed.
	 * When you are closer to the LevelOfDetail model,
	 * a more detailed child is displayed. As you move
	 * further away, a less detailed model is chosen.
	 *
	 * The range for each child is a floating point number and
	 * the LOD options define their interpretation.
	 * The ranges must be in ascending order and the children
	 * this model should be kept in the same order as their
	 * corresponding ranges. The base implementation provides these behaviors:
	 * @code
	 *	CAMERA_DISTANCE		Range value is the distance between the
	 *						center of the LOD model and the camera.
	 *						The first child whose range is greater or equal
	 *						to the distance from the camera is displayed.
	 *	SCREEN_SIZE_RATIO	Range value is the ratio of the screen size
	 *						of the LOD model to the camera viewport size.
	 *						The first child whose range is greater or equal
	 *						to the screen size ratio is displayed.
	 *	AUTOMATIC			Automatically calculate the ranges.
	 * @endcode
	 *  
	 * @see BillBoard
	 */
class LevelOfDetail : public Model
{
public:
	VX_DECLARE_CLASS(LevelOfDetail);

	LevelOfDetail();

	FloatArray*		GetRanges() const;
	void			SetRanges(FloatArray*);	//!< Set distance range array.
	void			AddRange(float dist);		//!< Add a distance to the range array.
	void			SetOptions(int opts);		//!< Set LOD range options.
	int				GetOptions() const;

// Overrides
	virtual int		Cull(const Matrix*, Scene*);
	virtual	void	Render(Scene*);
	virtual bool	Copy(const SharedObj*);
	virtual bool	Do(Messenger& s, int op);
	virtual int		Save(Messenger&, int) const;
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	static	bool	DoRanges;	//!< Enable/disable level of detail ranges.
	static	float	Scale;		//!< Apply global scaling factor to all ranges.

	/*!
	 * @brief LOD options.
	 * @see SetOptions
	 */
	enum
	{
		CAMERA_DISTANCE = 0,		//!< user camera distance to select level
		SCREEN_SIZE_RATIO = 1,		//!< use screen size projection to select level
		AUTOMATIC = 2				//!< automatically compute ranges
	};

	/*
	 * LevelOfDetail::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		LOD_AddRange = Model::MOD_NextOp,
		LOD_SetRanges,
		LOD_SetOptions,
		LOD_NextOp = Model::MOD_NextOp + 10,
	};

protected:
	void			ComputeRanges();

	int32				m_Options;
	Ref<Model>		m_SelectedChild;
	Ref<FloatArray>	m_Ranges;
};

inline LevelOfDetail::LevelOfDetail() : Model() { m_Options = AUTOMATIC; }

inline FloatArray* LevelOfDetail::GetRanges() const
	{ return m_Ranges; }

inline int LevelOfDetail::GetOptions() const
	{ return m_Options; }

} // end Vixen