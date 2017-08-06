#pragma once

namespace Vixen {

inline Col4::Col4(float red, float green, float blue, float alpha)
	{ r = red; g = green; b = blue; a = alpha; }

inline Col4::Col4(const Col4& c)
	{ r = c.r; g = c.g; b = c.b; a = c.a; }

inline Col4::Col4(const float* data)
	{ r = data[0]; g = data[1]; b = data[2]; a = data[3]; }

inline Col4::Col4(Color c)
	{ *this = c; }

inline Col4::Col4()
	{ }

inline Col4& Col4::operator=(const Col4& c)
	{ r = c.r; g = c.g; b = c.b; a = c.a; return *this; }

inline Col4& Col4::operator=(Color c)
{
	r = c.r / 255.0f;
	g = c.g / 255.0f;
	b = c.b / 255.0f;
	a = c.a / 255.0f;
	return *this;
}

inline void Col4::Set(float red, float green, float blue, float alpha)
	{ r = red; g = green; b = blue; a = alpha; }


/*!
 * @fn Col4 Col4::operator+=(const Col4& v)
 *
 * Colors are added by summing individual components:
 *	r += c.r, g += c.g, b += c.b, a += c.a
 *
 * @see Col4::operator+= Col4::operator-
 */
inline Col4& Col4::operator+=(const Col4& c)
	{ r += c.r; g += c.g; b += c.b; a += c.a; return *this; }

/*!
 * @fn Col4 Col4::operator+(const Col4& v) const
 *
 * Creates a new color by summing individual components:
 *	r + c.r, g + c.g, b + c.b, a + c.a
 *
 * @see Col4::operator+= Col4::operator-
 */
inline Col4 Col4::operator+(const Col4& c) const
	{ return Col4(r + c.r, g + c.g, b + c.b, a + c.a); }

/*!
 * @fn Col4 Col4::operator-(const Col4& v) const
 *
 * Creates a new color by subtracting individual components:
 *	r - c.r, g - c.g, b - c.b, a - c.a
 *
 * @see Col4::operator-= Col4::operator+
 */
inline Col4 Col4::operator-(const Col4& c) const
	{ return Col4(r - c.r, g - c.g, b - c.b, a - c.a); }

/*!
 * @fn Col4 Col4::operator-=(const Col4& v)
 *
 * Colors are added by summing individual components:
 *	r -= c.r, g -= c.g, b -= c.b, a -= c.a
 *
 * @see Col4::operator+= Col4::operator-
 */
inline Col4& Col4::operator-=(const Col4& c)
	{ r -= c.r; g -= c.g; b -= c.b; a -= c.a; return *this; }

/*!
 * @fn Col4& Col4::operator*=(float f)
 *
 * Multiplies each component by a constant
 *	r *= f, g *= f, b *= f, a *= f
 *
 * @see Col4::operator*
 */
inline Col4& Col4::operator*=(float v)
	{ r *= v; g *= v; b *= v; a *= v; return *this; }

/*!
 * @fn Col4& Col4::operator*(float f) const
 *
 * Multiplies each component by a constant
 *	r * f, g * f, b * f, a * f
 *
 * @see Col4::operator*=
 */
inline Col4 Col4::operator*(float v) const
	{ return Col4(r * v, g * v, b * v, a * v); }

/*!
 * @fn Col4& Col4::operator/=(float f)
 *
 * Multiplies each component by a constant
 *	r /= f, g /= f, b /= f, a /= f
 *
 * @see Col4::operator*
 */
inline Col4& Col4::operator/=(float v)
	{ r /= v; g /= v; b /= v; a /= v; return *this; }

/*!
 * @fn Col4& Col4::operator/(float f) const
 *
 * Divides each component by a constant
 *	r / f, g / f, b / f, a / f
 *
 * @see Col4::operator*=
 */
inline Col4 Col4::operator/(float v) const
	{ return Col4(r / v, g / v, b / v, a / v); }

/*!
 * @fn bool Col4::operator==(const Col4& v) const
 *
 * Colors are equal if the individual components are equal.
 *
 * @returns \b true if colors the same, else \b false
 *
 * @see Col4::operator!=
 */
inline bool Col4::operator==(const Col4& c) const
{
	return (r == c.r) && (g == c.g) && (b == c.b) && (a == c.a);
}

/*!
 * @fn bool Col4::operator!=(const Col4& v) const
 *
 * Colors are different if the any of the components are different.
 *
 * @returns \b true if colors are different, else \b false
 *
 * @see Col4::operator==
 */
inline bool Col4::operator!=(const Col4& c) const
{
	return (r != c.r) || (g != c.g) || (b != c.b) || (a != c.a);
}

inline DebugOut& operator<<(DebugOut& s, const Col4& c)
	{ return s << c.r << " "  << c.g << " " << c.b << " " << c.a; }

inline Col4::operator uint32() const
{
	uint32 alpha = (uint32) (a * 255.0f);	
	uint32 red = (uint32) (r * 255.0f);	
	uint32 green = (uint32) (g * 255.0f);	
	uint32 blue = (uint32) (b * 255.0f);
	uint32 v = Color(red, green, blue, alpha);	
	return v;
}

inline Col4::Col4(uint32 v)
{
	Color tmp(v);
	r = float(tmp.r)  / 255.0f;
	g = float(tmp.g) / 255.0f;
	b = float(tmp.b) / 255.0f;
	a = float(tmp.a) / 255.0f;
}

inline void Col4::Set(uint32 v)
{
	Color tmp(v);
	r = float(tmp.r)  / 255.0f;
	g = float(tmp.g) / 255.0f;
	b = float(tmp.b) / 255.0f;
	a = float(tmp.a) / 255.0f;
}

inline Color::Color(int red, int green, int blue, int alpha)
	{ r = (char) red; g = (char) green; b = (char) blue; a = (char) alpha; }

inline Color::Color(const Color& c)
	{ r = c.r; g = c.g; b = c.b; a = c.a; }

inline Color::Color()
	{ }

inline Color::Color(const Col4& c)
	{ *this = c; }

inline Color& Color::operator=(const Color& c)
	{ r = c.r; g = c.g; b = c.b; a = c.a; return *this; }

inline Color& Color::operator=(const Col4& c)
{
	r = (char) (c.r * 255.0f);
	g = (char) (c.g * 255.0f);
	b = (char) (c.b * 255.0f);
	a = (char) (c.a * 255.0f);
	return *this;
}

inline void Color::Set(int red, int green, int blue, int alpha)
	{ r = (char) red; g = (char) green; b = (char) blue; a = (char) alpha; }


/*!
 * @fn Color Color::operator+=(const Color& v)
 *
 * Colors are added by summing individual components:
 *	r += c.r, g += c.g, b += c.b, a += c.a
 *
 * @see Color::operator+= Color::operator-
 */
inline Color& Color::operator+=(const Color& c)
	{ r += c.r; g += c.g; b += c.b; a += c.a; return *this; }

/*!
 * @fn Color Color::operator+(const Color& v) const
 *
 * Creates a new color by summing individual components:
 *	r + c.r, g + c.g, b + c.b, a + c.a
 *
 * @see Color::operator+= Color::operator-
 */
inline Color Color::operator+(const Color& c) const
	{ return Color(r + c.r, g + c.g, b + c.b, a + c.a); }

/*!
 * @fn Color Color::operator-(const Color& v) const
 *
 * Creates a new color by subtracting individual components:
 *	r - c.r, g - c.g, b - c.b, a - c.a
 *
 * @see Color::operator-= Color::operator+
 */
inline Color Color::operator-(const Color& c) const
	{ return Color(r - c.r, g - c.g, b - c.b, a - c.a); }

/*!
 * @fn Color Color::operator-=(const Color& v)
 *
 * Colors are added by summing individual components:
 *	r -= c.r, g -= c.g, b -= c.b, a -= c.a
 *
 * @see Color::operator+= Color::operator-
 */
inline Color& Color::operator-=(const Color& c)
	{ r -= c.r; g -= c.g; b -= c.b; a -= c.a; return *this; }

/*!
 * @fn bool Color::operator==(const Color& v) const
 *
 * Colors are equal if the individual components are equal.
 *
 * @returns \b true if colors the same, else \b false
 *
 * @see Color::operator!=
 */
inline bool Color::operator==(const Color& c) const
{
	return *((uint32*) &r) == *((uint32*) &c.r);
}

/*!
 * @fn bool Color::operator!=(const Color& v) const
 *
 * Colors are different if the any of the components are different.
 *
 * @returns \b true if colors are different, else \b false
 *
 * @see Color::operator==
 */
inline bool Color::operator!=(const Color& c) const
{
	return *((uint32*) &r) != *((uint32*) &c.r);
}

inline Color::operator uint32() const
{	
#if defined(VIXEN_OGL) || defined(VIXEN_GLES2)
	uint32 v = (a << 24) | (b << 16) | (g << 8) | r;
#else
	uint32 v = (r << 24) | (g << 16) | (b << 8) | a;
#endif
	return v;
}

inline Color::Color(uint32 v)
{
	*this = v;
}

/*
 * Format of DirectX RGBA colors is
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    red      |      green      |     blue     |     alpha      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Format of OpenGL RGBA colors is
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    alpha      |     blue      |     green     |      red      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
inline Color& Color::operator=(uint32 v)
{
#if defined(VIXEN_OGL) || defined(VIXEN_GLES2)
	a = (char) (v >> 24);
	r = (char) (v >> 16);
	g = (char) (v >> 8);
	b = (char) (v & 0xff);
#else
	a = (char) (v >> 24);
	b = (char) (v >> 16);
	g = (char) (v >> 8);
	r = (char) (v & 0xff);
#endif
	return *this;
}

} // end Vixen