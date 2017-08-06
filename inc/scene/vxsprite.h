#pragma once

namespace Vixen {

class Texture;
class ImageSwitch;
class TextGeometry;

/*!
 * @class Sprite
 * @brief flat, rectangular object that always faces the viewer.
 *
 * A Sprite usually implemented as a polygon (usually 2 triangles)
 * with a texture decal mapped onto it. If the texture selected
 * every frame is varied over time, the sprite will appear animated.
 * Sprites can also be used to display text.
 *
 * The sprite size is specified as a proportion of
 * the camera view volume. A sprite with an upper left corner of
 * 0,0 and a lower right corner of 1,1 will cover the whole screen
 * when no transformation matrix is applied. A sprite will change
 * position based on its transformation matrix but it will not change
 * size or orientation.
 *
 * Unlike other models, a sprite does not inherit its parents
 * transformation matrix. This allows you to move the sprite around
 * the view volume but it will display the same no matter what object it is
 * attached to. You can use either the sprite's corner to place it
 * within the view volume for a text overlay or splash screen.
 *
 * The texture associated with the Sprite is kept in the appearance
 * used by the sprite geometry. The Image in this appearance can be
 * a single texture, or it can be an ImageSwitch.
 *
 * @par Example
 * @code
 *	// make a text sprite covering the upper quadrant of the screen
 *	Sprite* texanim = new Sprite;
 *	sprite->SetUpperLeft(0.5, 0.5);
 *	sprite->SetText("hello World");
 * @endcode
 *
 * Properties:
 *	SetUpperLeft	upper left corner of sprite
 *	SetLowerRight	lower right corner of sprite
 *
 * @see SimpleShape BillBoard TextGeometry ImageSwitch
 */
class Sprite : public SimpleShape
{
public:
	VX_DECLARE_CLASS(Sprite);

	Sprite();
//! Get upper left corner of sprite bounding rectangle.
	Vec2			GetUpperLeft() const	{ return mUpperLeft; }
//! Get lower right corner of sprite bounding rectangle.
 	Vec2			GetLowerRight() const	{ return mLowerRight; }
//! Set upper left corner of sprite bounding rectangle.
	virtual void	SetUpperLeft(const Vec2& ul);
	virtual void	SetUpperLeft(float x, float y)	{ SetUpperLeft(Vec2(x, y)); }
//! Set lower right corner of sprite bounding rectangle.
	virtual void	SetLowerRight(const Vec2& lr);
	virtual void	SetLowerRight(float x, float y)	{ SetLowerRight(Vec2(x, y)); }

	bool			CalcMatrix(Matrix* trans, Scene* scene) const;
	virtual bool	Copy(const SharedObj*);
	virtual bool	Do(Messenger& s, int op);
	virtual int		Save(Messenger&, int) const;
	virtual void	Render(Scene*);
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*
	 * Sprite::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		SPRITE_SetImage = SHAPE_NextOp,
		SPRITE_SetUpperLeft,
		SPRITE_SetLowerRight,
		SPRITE_SetText,
		SPRITE_MakeGeometry,
		SPRITE_NextOp
	};

protected: 
	Geometry*	MakeImageGeometry();
	void		UpdateGeometry(Scene*);

//	Data
private:
	int32	mChanged;
	Box3	mLastViewVolume;
	Vec2	mUpperLeft;		// Parameterized screen coords - upperLeft
	Vec2	mLowerRight;	// Parameterized screen coords - lowerRight
};

} // end Vixen