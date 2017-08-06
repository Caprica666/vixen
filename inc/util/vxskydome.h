#pragma once

namespace Vixen {

/*!
 * @class DayDome
 * @brief A shape meant to simulate a daytime sky.
 *
 * @see SkyDome StarDome
 *
 * @author Jeremiah Williamson
 */
class DayDome : public Shape
{
public:
	VX_DECLARE_CLASS(DayDome);
	DayDome();
	~DayDome();
//! Create daydome geometry
	void	Create(int depth, const char *sun_texture = NULL);

//! sets the time of day and updates day dome colors
	void	SetTimeOfDay(int hrs, int mins, int secs);
//! sets the beginning, end, and duration of dawn
	void	SetDawnTimes(int start_hr, int start_min, int start_sec,
						 int end_hr, int end_min, int end_sec);
//! sets the beginning, end, and duration of dusk
	void	SetDuskTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec);
//! Set radius of daydome
	void	SetRadius(float radius);
//! sets the color of the sun shape in the scene.
	void	SetSunColor(const Col4& c);
//! Sets the color of sunset on the day dome.
	void	SetSunset(const Col4& c);
//! Return the sunset color
	const Col4& GetSunset() const		{ return SunsetCol; }
//! Sets the day dome horizon color
	void 	SetHorizon(const Col4& horizon);
//! Return the horizon color
	const Col4& GetHorizon() const	{ return DayHorizonCol; }
//! Sets the day dome azimuth color
	void		SetAzimuth(const Col4& azimuth);
//! Return the azimuth color
	const Col4& GetAzimuth() const	{ return DayAzimuthCol; }
//! Sets the maximum sunset angle
	void	SetMaxSunsetAngle(float sunset_angle);
	void	SetSunModifier(float sunpct);
	void	SetInterpCutoff(float cutoff);
//! Return the brightness of the sunlight
	float	GetBrightness()				{ return m_fademod; }

protected:
	void	ApplyDayColors();	// calculates colors depending on time of day

	Col4 	DayHorizonCol, DayAzimuthCol, SunsetCol;
	float	Radius;
	float	Cutoff, MaxSunsetAngle, SunTimePctMod;

	int     TimeOfDay, MaxTimeOfDay;
	int		DuskStartTime, DuskEndTime, DawnStartTime, DawnEndTime;
	float	DawnDuration, DuskDuration;

	float	DawnSeqStart, DuskSeqEnd;
	float	DuskMidSeq, DuskMidSeqTime, DawnMidSeq, DawnMidSeqTime;

	int		NumVertices;
	Vec3	*SphVertices;

	float	m_fademod;
	void	ConvertVertices();

	Ref<Shape> m_pSunShape;
	Ref<Appearance> m_SunApp;
};

inline void DayDome::SetHorizon(const Col4& horizon)
{ DayHorizonCol = horizon; ApplyDayColors(); }

inline void DayDome::SetAzimuth(const Col4& azimuth)
{ DayAzimuthCol = azimuth; ApplyDayColors(); }

inline void DayDome::SetSunset(const Col4& sunset)
{ SunsetCol = sunset; }

inline void DayDome::SetMaxSunsetAngle(float sunset_angle)
{ MaxSunsetAngle = sunset_angle; }

inline void DayDome::SetInterpCutoff(float cutoff)
{ Cutoff = cutoff; }

inline void DayDome::SetSunModifier(float sunpct)
{ SunTimePctMod = sunpct; }


/*!
 * @class StarDome
 * @brief A shape meant to simulate a nighttime sky.
 *
 * @see DayDome SkyDome
 *
 * @author Jeremiah Williamson
 */
class StarDome : public Shape
{
public:
	VX_DECLARE_CLASS(StarDome);
//! Make empty stardome with default settings.
	StarDome();
//! Destroy stardome and the resources it uses.
	~StarDome();
//! Create stardome geometry	
	void	Create(const char *starfile, float brightness, const char *moon_texture = NULL);
//! Sets the time of day and updates stars
	void	SetTimeOfDay(int hrs, int mins, int secs);
//! Sets the beginning, end, and duration of dawn
	void	SetDawnTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec);
//! Sets the beginning, end, and duration of dusk
	void	SetDuskTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec);
//! Sets the path of the file containing the star data
	void	SetStarFile(const char *starpath);
//! Sets the radius of the star dome.
	void	SetRadius(float radius);

private:
	struct Star
	{
		float 	ra;		// Right Ascension 
		float	dec;	// Declination
		float	mag;	// Magnitude
		float	relmag;
		Vec3	cartpt;	// Cartesian coordinates
	};

	float	Radius;
	float	RelativeBrightness;
	int     TimeOfDay, MaxTimeOfDay;
	int		DuskStartTime, DuskEndTime;
	int		DawnStartTime, DawnEndTime;
	float	DawnPct, DuskPct;
	float	DawnDuration, DuskDuration;

	float	HighMag, LowMag, MagRange;

	// Star array and the number of stars in the array
	int		NumStars;
	Star*	Starfield;
	String	StarFile;

	// Read star data from file - returns number of stars read into array 
	void	Read_Stardata(const char *starfile);
	void	Convert_Starcoord(Star *star);
	
	void	AddStars(VertexArray*);
	void	FadeStars();
};


/*!
 * @class SkyDome
 * @brief A model meant to simulate the day and night sky.
 *
 * It contains both a DayDome and a StarDome. 
 *
 * @see DayDome StarDome
 *
 * @author Jeremiah Williamson
 */
class SkyDome : public Model
{
public:
	VX_DECLARE_CLASS(SkyDome);
//! Create empty sky dome with default settings.
	SkyDome();
//! Create and initialize both day and night geometry.
	void		Create(const char *starfile = NULL, int depth = 0,
						const char *sun_texture = NULL, const char *moon_texture = NULL);
//! set the radius of the star dome
	void		SetRadius(float radius);
//! Gets the sky dome radius.
	float		GetRadius() const	{ return Radius; }
//! sets the time of day and updates sky dome
	void		SetTimeOfDay(int hrs, int mins, int secs);
//! sets the color of the sun shape in the scene.
	void		SetSunColor(const Col4 &color);
//! Sets the color of sunset on the day dome.
	void		SetSunset(const Col4& sunset);
//! Return the sunset color
	const Col4& GetSunset() const		{ return m_DayDome->GetSunset(); }
//! Sets the day dome horizon color
	void		SetHorizon(const Col4& horizon);
//! Return the horizon color
	const Col4& GetHorizon() const	{ return m_DayDome->GetHorizon(); }
//! Sets the day dome azimuth color
	void		SetAzimuth(const Col4& azimuth);
//! Return the azimuth color
	const Col4& GetAzimuth() const	{ return m_DayDome->GetAzimuth(); }
//! sets the beginning, end, and duration of dawn
	void		SetDawnTimes(int start_hr, int start_min, int start_sec,
						     int end_hr, int end_min, int end_sec);
//! sets the beginning, end, and duration of dusk
	void		SetDuskTimes(int start_hr, int start_min, int start_sec,
							 int end_hr, int end_min, int end_sec);
//! Establish a scene light to use as the sun
	void		SetSunLight(Light *light)	{ m_pSunLight = light; }
//! sets the time interpolation value
	void		SetTime(float t);

	void		SetMaxSunsetAngle(float sunset_angle);
	void		SetInterpCutoff(float cutoff);
	DayDome*	GetDayDome()				{ return m_DayDome; }
	StarDome*	GetStarDome()				{ return m_StarDome; }

	static Appearance*	MakeTextureAppearance(const TCHAR* texfile);

protected:
	Ref<DayDome>	m_DayDome;
	Ref<StarDome>	m_StarDome;
	Ref<Light>	m_pSunLight;

private:
	float	Radius;
	int     TimeOfDay, MaxTimeOfDay;
	int		DuskStartTime, DuskEndTime;
	int		DawnStartTime, DawnEndTime;
	float	HighMag, LowMag, MagRange;
};

} // end Vixen