/*!
 * @file vxcol.h
 * @brief Color representations.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxmath.h
 */
#pragma once

namespace Vixen {

class Messenger;
class Color;

/*!
 * @class Col4
 * @brief Color represented in terms of red, green, blue and alpha
 * stored as 4 single-precision floating point numbers.
 *
 * @ingroup vixen
 * @see Vec4 Color
 */
class Col4
{
public:
	float	r;	//!< red component of color, 0 to 1
	float	g;	//!< green component of color, 0 to 1
	float	b;	//!< blue component of color, 0 to 1
	float	a;	//!< alpha component of color, 0 to 1

//! @name Construction
//@{
	Col4(float r, float g, float b, float a = 1);
	Col4(const Col4&);				//!< Copy source color into this color.
	Col4();							//!< Initialize by zeroing all components.
	Col4(Color);					//!< Initialize from 4 byte format.
	Col4(uint32);						//!< Initialize from 4 byte format.
	Col4(const float* floatArray);	//!< Initialize color from float array.
	operator Color() const;			//!< Convert color to 4 byte format.
	operator uint32() const;			//!< Convert color to 4 byte format.
//@}

//! @name Operators
//@{
	const	float&	operator[](int) const;	//!< Access color as array.
	float&	operator[](int);				//!< Access color as array.
	Col4& operator=(const Col4&);		//!< Copy into this color.
	Col4& operator=(Color);				//!< Copy into this color.
	Col4	operator-() const;				//!< Return negative of this color.
	Col4	operator-(const Col4&) const;	//!< Subtract two colors.
	Col4&	operator-=(const Col4&);		//!< Subtract from this color.
	Col4	operator+(const Col4&) const;	//!< Add two colors.
	Col4&	operator+=(const Col4&);		//!< Add to this color.
	Col4	operator*(float) const;			//!< Scale color by a constant.
	Col4&	operator*=(float);				//!< Multiply this color by a constant.
	Col4&	operator/=(float);				//!< Divide this color by a constant.
	Col4	operator/(float) const;			//!< Scale color by a constant.
	bool	operator==(const Col4&) const;	//! Determine if two colors are equal.
	bool	operator!=(const Col4&) const;	//! Determine if two colors are not equal.
//@}
		
//! @name Update
//@{
	void	Set(uint32);					//!< Set color from 4 byte format.
	void	Set(float R, float G, float B, float A = 1); //!< Set this color from R, G, B and A components.

//@}
};

DebugOut& operator<<(DebugOut&, const Col4&);

/*!
 * @fn Messenger& operator<<(Messenger& pipe, const Col4& c)
 * @param disp	messenger to output vector to
 * @param c		color to write
 *
 * Outputs a color to the messenger.
 * The X, Y and Z components are written as 32 bit floating point numbers.
 */
extern Messenger& operator<<(Messenger& d, const Col4& c);
extern Messenger& operator>>(Messenger& d, Col4& c);

/*!
 * @class Color
 * @brief Color represented in terms of red, green, blue and alpha
 * stored as 8 bit integers in a 32 bit word.
 *
 * When rendering with DirectX it matches D3DCOLOR.
 * Under OpenGL, the red and blue are reversed to match OpenGL color format.
 *
 * @ingroup vixen
 * @see Vec4 Col4
 */
class Color
{
public:
#if defined(VIXEN_OGL)
	char	r;
	char	g;
	char	b;
	char	a;
#else
	char	b;	//!< alpha component of color, 0 to 255
	char	g;	//!< blue component of color, 0 to 255
	char	r;	//!< green component of color, 0 to 255
	char	a;	//!< red component of color, 0 to 255
#endif

//! @name Construction
//@{
	Color(int r, int g, int b, int a = -1);
	Color(const Color&);	//!< Copy source color into this color.
	Color(const Col4&);		//!< Copy 4 word float color into this color.
	Color();				//!< Initialize by zeroing all components.
	Color(uint32);			//!< Initialize from 32 bit integer.
	operator uint32() const;//!< Convert color to 32 bit integer.
	operator Col4() const;	//!< Convert color to 4 word float representation.
//@}

//! @name Operators
//@{
	const char&	operator[](int) const;		//!< Access color as array.
	char&	operator[](int);				//!< Access color as array.
	Color&	operator=(const Color&);		//!< Copy into this color.
	Color&	operator=(const Col4&);			//!< Copy 4 word float color into this color.
	Color&	operator=(uint32 v);			//!< Copy integer color into this one.
	Color	operator-() const;				//!< Return negative of this color.
	Color	operator-(const Color&) const;	//!< Subtract two colors.
	Color&	operator-=(const Color&);		//!< Subtract from this color.
	Color	operator+(const Color&) const;	//!< Add two colors.
	Color&	operator+=(const Color&);		//!< Add to this color.
	bool	operator==(const Color&) const;	//! Determine if two colors are equal.
	bool	operator!=(const Color&) const;	//! Determine if two colors are not equal.
//@}
		
//! @name Update
//@{
	void		Set(int R, int G, int B, int A = 1); //!< Set this color from R, G, B and A components.
//@}
};

} // end Vixen