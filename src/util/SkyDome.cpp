#include "vixen.h"
#include "vxutil.h"
#include "util/vxskydome.h"

// minimum and maximum ambient light values
#define MIN_AMB		0.0f
#define MAX_AMB		1.0f
#define AMB_RANGE	(MAX_AMB-MIN_AMB)

// maximum directional light intensity
#define MAX_INT		1.0f

// only show stars up to this magnitude
#define MAX_MAGNITUDE	5.8f

#define SQR(x)	x * x
#define TWO_PI (PI * 2.0f)
#define HALF_PI	(PI * 0.5f)
#define QUARTER_PI	(PI * 0.25f)

// Radians <-> Degrees Macros, single angle
#define RAD_TO_DEG(x) (x * (180.0f/PI))
#define DEG_TO_RAD(x) (x * (PI/180.0f))


// Coordinate change macros for a point class
// Here x = rho, y = theta, z = phi 
#define rho		x
#define theta	y
#define phi		z

#define PT_CART_TO_SPHERE(A, B) {\
	B.rho = sqrtf(SQR(A.x) + SQR(A.y) + SQR(A.z));\
	B.theta = atan2f(A.y, A.x);\
	B.phi = atan2f(A.z, sqrt(SQR(A.x) + SQR(A.y)));\
}

#define PT_SPHERE_TO_CART(A, B) {\
	B.x = A.rho * cosf(A.phi) * sinf(A.theta);\
	B.z = A.rho * cosf(A.phi) * cosf(A.theta);\
	B.y = A.rho * sinf(A.phi);\
}

#define TIME_TO_INT(hr, min, sec) ((hr * 60 + min) * 60 + sec)

VX_IMPLEMENT_CLASSID(DayDome, Shape, VX_DayDome);
VX_IMPLEMENT_CLASSID(StarDome, Shape, VX_StarDome);
VX_IMPLEMENT_CLASSID(SkyDome, Model, VX_SkyDome);

static int dawn_start = TIME_TO_INT(4, 30, 0);
static int dawn_middle = TIME_TO_INT(5, 30, 0);
static int dawn_end = TIME_TO_INT(6, 30, 0);
static int dusk_start = TIME_TO_INT(17, 30, 0);
static int dusk_middle = TIME_TO_INT(18, 30, 0);
static int dusk_end = TIME_TO_INT(19, 30, 0);

static int sunrise = TIME_TO_INT(6, 0, 0);
static int sunset = TIME_TO_INT(18, 0, 0);
static int sunsize = TIME_TO_INT(0, 16, 0);

SkyDome::SkyDome() : Model()
{
	m_DayDome = new DayDome();
	m_StarDome = new StarDome();
	SetRadius(1000.0f);
	MaxTimeOfDay = TIME_TO_INT(24, 0, 0);
	SetDawnTimes(5, 0, 0, 7, 0, 0);
	SetDuskTimes(17, 0, 0, 19, 0, 0);
	Append(m_DayDome);
	Append(m_StarDome);
	SetTimeOfDay(12, 0, 0);
}

		
/*!
 * @fn void SkyDome::Create(const char *starfile, int depth,
 *	const char *sun_texture, const char *moon_texture)
 * @param starfile	name of file with star data, if NULL no star dome geometry is made
 * @param depth	complexity of day dome
 * @param sun_texture	name of file with the sun texture
 * @param moon_texture	name of file with the moon texture
 *
 * Creates a complete SkyDome, which includes a DayDome and a StarDome.
 * The radius and time of day are used to initialize the state of the skydome.
 * These default to 1000 and noon, respectively. If the sun or moon textures
 * are omitted, these shapes are not simulated
 *
 * @see StarDome::Create DayDome::Create SkyDome::SetRadius SkyDome::SetTimeOfDay
 */
void SkyDome::Create(const char *starfile, int depth,
					 const char *sun_texture, const char *moon_texture)
{
	// Create the PSDayDome
	m_DayDome->Create(depth, sun_texture);
	m_DayDome->SetName("scene.daydome");

	// Create the PSStarDome
	if (starfile)
	{
		m_StarDome->Create(starfile, 2.0f, moon_texture);
		m_StarDome->SetName("scene.stardome");
	}
}


/*!
 * @fn void SkyDome::SetTimeOfDay(int hr, int min, int sec)
 * @param hr	hour of the day
 * @param min	minute in that hour
 * @param sec	second in that minute
 *
 * Sets time of day, updates sky color and sun's color and position as appropriate
 * also calls StarDome and DayDome equivalent functions
 *	
 * @see DayDome::SetTimeOfDay StarDome::SetTimeOfDay
 */
void SkyDome::SetTimeOfDay(int hr, int min, int sec)
{
	TimeOfDay = TIME_TO_INT(hr, min, sec); 

	// Pass along time of day information to the PSStarDome and PSDayDome
	m_StarDome->SetTimeOfDay(hr, min, sec);
	m_DayDome->SetTimeOfDay(hr, min, sec);

	// Determine which of the two domes are active according to time of day
	if (TimeOfDay >= DuskStartTime || TimeOfDay <= DawnEndTime) 
		m_StarDome->SetActive(true);
	else
		m_StarDome->SetActive(false);

	// set the direction and intensity of the sunlight
	if (m_pSunLight != NULL)
	{
		float angle = 2.0f * PI * ((float)TimeOfDay / (24 * 60 * 60));
		m_pSunLight->Reset();
		m_pSunLight->Rotate(Vec3(0,1,0), -PI/2.0f);
		m_pSunLight->Rotate(Vec3(0,0,1), angle + PI/2.0f);

		float ambient = 0.0f;

		// set intensity of ambient light based on time of day
		if (TimeOfDay > dusk_end || TimeOfDay < dawn_start)
		{
			// night
			ambient = MIN_AMB;
		}
		else if (TimeOfDay >= dawn_start && TimeOfDay <= dawn_end)
		{
			// dawn
			ambient = MIN_AMB + AMB_RANGE * (float)(TimeOfDay - dawn_start) / (dawn_end - dawn_start);
		}
		else if (TimeOfDay >= dawn_end && TimeOfDay <= dusk_start)
		{
			// day
			ambient = MAX_AMB;
		}
		else if (TimeOfDay >= dusk_start && TimeOfDay <= dusk_end)
		{
			// dusk
			ambient = MIN_AMB + AMB_RANGE * (float)(dusk_end - TimeOfDay) / (dawn_end - dawn_start);
		}

		Scene* scene = GetMainScene();
		float intensity = 0.0f;

		Col4 white(1.0f, 1.0f, 1.0f);
		Col4 yellow(1.0f, 0.6f, 0.4f);
		Col4 color;
		float fraction;

		scene->SetAmbient(Col4(ambient, ambient, ambient));
		// set intensity of sunlight based on time of day
		if (TimeOfDay > sunset+sunsize || TimeOfDay < sunrise-sunsize)
		{
			// night
			intensity = 0.0f;
		}
		else if (TimeOfDay >= sunrise-sunsize && TimeOfDay <= sunrise+sunsize)
		{
			// dawn
			fraction = (float)(TimeOfDay - (sunrise-sunsize)) / (sunsize*2.0f);
			color = yellow + ((white - yellow) * fraction);
			intensity = MAX_INT * fraction;
		}
		else if (TimeOfDay >= sunrise+sunsize && TimeOfDay <= sunset-sunsize)
		{
			// day
			color = white;
			intensity = MAX_INT;
		}
		else if (TimeOfDay >= sunset-sunsize && TimeOfDay <= sunset+sunsize)
		{
			// dusk
			fraction = (float)(sunset+sunsize - TimeOfDay) / (sunsize*2.0f);
			color = yellow + ((white - yellow) * fraction);
			intensity = MAX_INT * fraction;
		}

		m_DayDome->SetSunColor(color);
		color *= intensity;
		m_pSunLight->SetColor(color);
	}
}

/*!
 * @fn void SkyDome::SetDuskTimes(int start_hr, int start_min, int start_sec,
 *	int end_hr, int end_min, int end_sec)
 * @param start_hr		start hour of dusk
 * @param start_min		start minute in that hour
 * @param start_sec		start second in that minute
 * @param end_hr		end hour of the dusk
 * @param end_min		end minute in that hour
 * @param end_sec		end second in that minute
 *
 * @see SkyDome::SetDawnTimes DayDome::SetDuskTimes StarDome::SetDuskTimes
 */
void SkyDome::SetDuskTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec)
{
	DuskStartTime = TIME_TO_INT(start_hr, start_min, start_sec);
	DuskEndTime = TIME_TO_INT(end_hr, end_min, end_sec);

	m_DayDome->SetDuskTimes(start_hr, start_min, start_sec, end_hr, end_min, end_sec);
	m_StarDome->SetDuskTimes(start_hr, start_min, start_sec, end_hr, end_min, end_sec);
}

/*!
 * @fn void SkyDome::SetDawnTimes(int start_hr, int start_min, int start_sec,
 *	int end_hr, int end_min, int end_sec)
 * @param start_hr		start hour of dawn
 * @param start_min		start minute in that hour
 * @param start_sec		start second in that minute
 * @param end_hr		end hour of the dawn
 * @param end_min		end minute in that hour
 * @param end_sec		end second in that minute
 *
 * @see SkyDome::SetDawnTimes DayDome::SetDuskTimes StarDome::SetDuskTimes
 */
void SkyDome::SetDawnTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec)
{
	DawnStartTime = TIME_TO_INT(start_hr, start_min, start_sec);
	DawnEndTime = TIME_TO_INT(end_hr, end_min, end_sec);

	m_DayDome->SetDawnTimes(start_hr, start_min, start_sec, end_hr, end_min, end_sec);
	m_StarDome->SetDawnTimes(start_hr, start_min, start_sec, end_hr, end_min, end_sec);
}


/*!
 * @fn void SkyDome::SetHorizon(const Col4& horizon)
 * @param horizon	color of the horizon
 *
 * The horizon color will be the color of the daydome base
 * (largest part). It should also become the background color
 * for the scene.
 *
 * @see SkyDome::SetAzimuth
 */
void SkyDome::SetHorizon(const Col4& horizon)
{
	m_DayDome->SetHorizon(horizon);
}

/*!
 * @fn void SkyDome::SetAzimuth(const Col4& azimuth)
 * @param horizon	color of the azimuth base.
 *
 * The azimuth color will be the color of the daydome peak.
 *
 * @see SkyDome::SetHorizon
 */
void SkyDome::SetAzimuth(const Col4& azimuth)
{
	m_DayDome->SetAzimuth(azimuth);
}

/*!
 * @fn void SkyDome::SetSunset(const Col4& sunset)
 * @param sunset	color of the sunset
 *
 * if there is a DayDome, set the color of sunset for it
 *	
 * @see PSSkyDome::SetDayColors PSDayDome::SetSunsetColor
 */
void SkyDome::SetSunset(const Col4& sunset)
{
	m_DayDome->SetSunset(sunset);
}

/*!
 * @fn void SkyDome::SetMaxSunsetAngle(float sunset_angle)
 * @param sunset_angle maximum sunset angle (in radians)
 *
 * The sunset angle is measured from horizon to max of 90deg at azimuth .
 * It defaults to 30 degrees.
 */
void SkyDome::SetMaxSunsetAngle(float sunset_angle)
{
	m_DayDome->SetMaxSunsetAngle(sunset_angle);
}

void SkyDome::SetInterpCutoff(float cutoff)
{
	m_DayDome->SetInterpCutoff(cutoff);
}


/*!
 * @fn void SkyDome::SetRadius(float radius)
 * @param radius	radius of sky dome
 *
 * The radius of the day and star domes are set by scaling them
 * with their local matrices.
 *
 * @see PSStarDome::SetRadius PSDayDome::SetRadius
 */
void SkyDome::SetRadius(float radius)
{
	m_DayDome->SetRadius(radius);
	m_StarDome->SetRadius(radius);
}

int SkyDome::AddTextureAppearance(const char* texfile)
{
	Material *pTextured2Mat = new Material();
	Appearance *pTextured2 = new Appearance();
	Sampler*	pSampler = new Sampler(texfile);

	pTextured2Mat->SetSpecular(Col4(0.0f, 0.0f, 0.0f));
	pTextured2Mat->SetAmbient(Col4(0, 0, 0));
	pTextured2Mat->SetDiffuse(Col4(1, 1, 1, 1));
	pTextured2->SetMaterial(pTextured2Mat);
	pTextured2->Set(Appearance::CULLING, false);
	pTextured2->Set(Appearance::LIGHTING, false);
	pSampler->Set(Sampler::TRANSPARENCY, true);
	pSampler->Set(Sampler::TEXTUREOP, Sampler::REPLACE);
	pTextured2->SetSampler(0, pSampler);
	return pApp;
}

inline DayDome::DayDome() : Shape()
{
	SphVertices = NULL;

	// Set default horizon, azimuth and sunset colors
	DayHorizonCol.Set(0.5f, 1.0f, 1.0f);
	DayAzimuthCol.Set(0.3f, 0.3f, 0.9f);
	SunsetCol.Set(1.0f, 0.55f, 0.0f);
	SetMaxSunsetAngle(0.5f);
	SetSunModifier(0.33f);

	// Set default Interpolation cutoff
	SetInterpCutoff(0.3f);

	// Set Maximum rotational change dependent only on hours, minutes and secs
	MaxTimeOfDay = TIME_TO_INT(24, 0, 0);

	SetDawnTimes(5, 0, 0, 7, 0, 0);
	SetDuskTimes(17, 0, 0, 19, 0, 0);
}


DayDome::~DayDome()
{
	if (SphVertices)
	delete[] SphVertices;
}

/*!
 * @fn void DayDome::Create(int depth, const char* sun_texture)
 *
 * Creates daydome geometry, including a sun if the sun texture file is included
 *	
 * Args:
 * @param depth			daydome complexity (recursion depth for icosahedrom)
 * @param sun_texture	the file containing the sun texture
 *	
 * @see DayDome::SetRadius DayDome::SetTimeOfDay StarDome::Create SkyDome::Create
 */
void DayDome::Create(int depth, const char* sun_texture)
{
	TriMesh* SphereGeom = new TriMesh(VTX_Colors);
	Appearance* a = new Appearance();

	a->Set(Appearance::LIGHTING, false);
	SetGeometry(SphereGeom);

	// no appearance index, because we're using vertex colors instead
	XFGeoUtil::IcosaSphere(SphereGeom, 1.0f, depth, true);
	SetAppearance(a);

	NumVertices = SphereGeom->GetNumVtx();
	SphVertices = new Vec3[NumVertices];
	ConvertVertices();
	ApplyDayColors();

	Reset();
	Rotate(MOD_XAxis, -HALF_PI);
	Scale(Radius, Radius, Radius);

	GetMainScene()->SetBackColor(SunsetCol);
	if (sun_texture)
	{
		// Create sun
		m_pSunShape = new Shape();
		TriMesh *SunGeom = new TriMesh(VTX_TexCoords);

		m_SunApp = SkyDome::AddTextureAppearance(sun_texture);
		m_SunApp->GetSampler(0)->Set(Sampler::TEXTUREOP, Sampler::MODULATE);
		m_pSunShape->SetAppearance(m_SunApp);
		XFGeoUtil::Rect(SphereGeom, 0.22f, 0.22f);
		m_pSunShape->SetGeometry(SunGeom);
		m_pSunShape->SetAppearances(apps);
		m_pSunShape->SetName("scene.skydome.sun");

		// why does Z translation work here?  should be Y translation?!
		Matrix trans;
		trans.Translate(0.0f, 0.0f, 0.85f);
		(*SunGeom) *= trans;

		Append(m_pSunShape);
	}
}

/*!
 * @fn void DayDome::SetRadius(float radius)
 * @param radius	new daydome radius
 *
 * Updates the local matrix to scale the day dome geometry  to the given radius.
 *
 * @see SkyDome::SetRadius DayDome::Create
 */
void DayDome::SetRadius(float radius)
{
	Radius = radius;
	Reset();
	Size(radius, radius, radius);
	Rotate(MOD_XAxis, -HALF_PI);
}

/*!
 * @fn void DayDome::SetTimeOfDay(int hr, int min, int sec)
 * @param hr	hour of the day
 * @param min	minute in that hour
 * @param sec	second in that minute
 *
 * Sets time of day, updates daydome colors and sun color.
 *	
 * @see PSSkyDome::SetTimeOfDay PSStarDome::SetTimeOfDay
 */
void DayDome::SetTimeOfDay(int hr, int min, int sec)
{	
	TimeOfDay = TIME_TO_INT(hr, min, sec); 

	// recolor vertices during dusk and dawn sequences
	if ((TimeOfDay >= DawnStartTime && TimeOfDay <= DawnEndTime) ||
		(TimeOfDay >= DuskStartTime && TimeOfDay <= DuskEndTime))
		ApplyDayColors();

	// determine sun color
	if (m_pSunShape != NULL)
	{
		// move sun: Set the right transformation for the time of day
		m_pSunShape->Reset();

		Scale(Radius, Radius, Radius);//
//		m_pSunShape->Rotate(TRANS_XAxis, PI/2.0f);
//		m_pSunShape->Turn(TRANS_YAxis, PI/2.0f);

		// should probably deviate 20 degrees from the zenith..
//		m_pSunShape->Rotate(TRANS_XAxis, -(0.4f * PI));//

		// rotate to move across sky
		m_pSunShape->Turn(Vec3(0,1,0), -((float)TimeOfDay/MaxTimeOfDay) * PI * 2.0f - (PI));
	}
}

/*!
 * @fn void DayDome::SetSunColor(Col4& col)
 * @param col	color of the sun
 *
 * Set the color of the sun object in the scene.
 *	
 * @see SkyDome::SetDayColors DayDome::SetSunsetColor
 */
void DayDome::SetSunColor(const Col4 &color)
{
	if (m_SunApp != NULL)
	{
		Material *mat = m_SunApp->GetMaterial();
		mat->SetDiffuse(color);
	}
}

void DayDome::ConvertVertices()
{	
	Vec3 p, psph;
	int i = 0;

	Mesh *geo = (Mesh*) GetGeometry();
	if (geo == NULL) return;

	VertexPool::Iter  iter(geo->GetVertices()); 
	while (iter.Next())
	{
		Vec3 p = *iter.GetLoc();
		PT_CART_TO_SPHERE(p, psph);
		SphVertices[i++] = psph;
	}
}

#define NITE_GLO 0.1f

void DayDome::ApplyDayColors()
{
	Col4 vtxcol;
	Vec3 p, psph;
	float phipct, phipct_cut, thetapct, sunpct;
	int i = 0;

	Mesh *geo = (Mesh *) GetGeometry();
	if (geo == NULL) return;

	VertexPool::Iter  iter(geo->GetVertices());

	float duskpct = NITE_GLO + (1.0f - NITE_GLO) * (float)(DuskEndTime - TimeOfDay)/DuskDuration;
	float dawnpct = NITE_GLO + (1.0f - NITE_GLO) * (float)(TimeOfDay - DawnStartTime)/DawnDuration;
	float midseq, midseqtime, midseqpct;

	if (TimeOfDay >= DuskStartTime && TimeOfDay <= DuskEndTime) 
	{
		// dusk
		m_fademod = duskpct;
		midseq = DuskMidSeq;
		midseqtime = DuskMidSeqTime;
		midseqpct = fabsf(midseqtime - (float)TimeOfDay)/midseq;
	}
	else if (TimeOfDay >= DawnStartTime && TimeOfDay <= DawnEndTime)
	{
		// dawn
		m_fademod = dawnpct;
		midseq = DawnMidSeq;
		midseqtime = DawnMidSeqTime;
		midseqpct = fabsf(midseqtime - (float)TimeOfDay)/midseq;
	}
	else if (TimeOfDay <= DawnStartTime || TimeOfDay >= DuskEndTime)
	{
		m_fademod = NITE_GLO;
	}
	else
		// day
		m_fademod = 1.0f;


	// Set day colors
	while (iter.Next())
	{
		p = *iter.GetLoc();
		psph = SphVertices[i++];

		phipct = fabsf(HALF_PI - fabsf(psph.phi))/HALF_PI;
		thetapct = (HALF_PI - fabsf(psph.theta))/HALF_PI;

		// Ordering of color layering is important!!!
		// Color is interpolated along phi from 0 to Cutoff
		if ((1.0f - phipct) <= Cutoff)
		{
			phipct_cut = (1.0f - phipct)/Cutoff;
			vtxcol = (DayHorizonCol * (1.0f - phipct_cut)) + (DayAzimuthCol * phipct_cut);
		}
		else
		{
			vtxcol = DayAzimuthCol;
		}
		vtxcol *= m_fademod;

		if (TimeOfDay >= DuskStartTime && TimeOfDay <= DuskSeqEnd)
		{
			if (((1.0f - phipct) <= MaxSunsetAngle) && (fabsf(psph.theta) > HALF_PI))
			{
				phipct_cut = (1.0f - phipct)/MaxSunsetAngle;
				sunpct = (1.0f - phipct_cut) * (1.0f - midseqpct) * (-thetapct);
				vtxcol = (vtxcol * (1.0f - sunpct)) + (SunsetCol * sunpct);
			}
		}
		else if (TimeOfDay >= DawnSeqStart && TimeOfDay <= DawnEndTime)
		{	
			if (((1.0f - phipct) <= MaxSunsetAngle) && (fabsf(psph.theta) <= HALF_PI))
			{
				phipct_cut = (1.0f - phipct)/MaxSunsetAngle;
				sunpct = (1.0f - phipct_cut) * (1.0f - midseqpct) * thetapct;
				vtxcol = (vtxcol * (1.0f - sunpct)) + (SunsetCol * sunpct);
			}
		}
		iter.SetColor(vtxcol);
	}
}

/*!
 * @fn void DayDome::SetDuskTimes(int start_hr, int start_min, int start_sec,
 *	int end_hr, int end_min, int end_sec)
 * @param start_hr		start hour of dusk
 * @param start_min		start minute in that hour
 * @param start_sec		start second in that minute
 * @param end_hr		end hour of the dusk
 * @param end_min		end minute in that hour
 * @param end_sec		end second in that minute
 *
 * @see PSDayDome::SetDawnTimes PSSkyDome::SetDuskTimes PSStarDome::SetDuskTimes
 */
void DayDome::SetDuskTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec)
{
	DuskStartTime = TIME_TO_INT(start_hr, start_min, start_sec);
	DuskEndTime = TIME_TO_INT(end_hr, end_min, end_sec);
	DuskDuration = (float)(DuskEndTime - DuskStartTime);
	DuskSeqEnd = (float)DuskEndTime - (DuskDuration * SunTimePctMod);
	DuskMidSeq = (DuskSeqEnd - (float)DuskStartTime)/2.0f;
	DuskMidSeqTime = DuskSeqEnd - DuskMidSeq;
}

/*!
 * @fn void DayDome::SetDawnTimes(int start_hr, int start_min, int start_sec,
 *	int end_hr, int end_min, int end_sec)
 * @param start_hr		start hour of dawn
 * @param start_min		start minute in that hour
 * @param start_sec		start second in that minute
 * @param end_hr		end hour of the dawn
 * @param end_min		end minute in that hour
 * @param end_sec		end second in that minute
 *
 * @see PSDayDome::SetDuskTimes PSSkyDome::SetDawnTimes
 */
void DayDome::SetDawnTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec)
{
	DawnStartTime = TIME_TO_INT(start_hr, start_min, start_sec);
	DawnEndTime = TIME_TO_INT(end_hr, end_min, end_sec);
	DawnDuration = (float)(DawnEndTime - DawnStartTime);
	DawnSeqStart = (float)DawnStartTime + (DawnDuration * SunTimePctMod);
	DawnMidSeq = (float)(DawnEndTime - DawnSeqStart)/2.0f;
	DawnMidSeqTime = (float)DawnEndTime - DawnMidSeq;
}

StarDome::StarDome() : Shape()
{
	Starfield = NULL;
}

StarDome::~StarDome()
{
	if (Starfield)
		delete[] Starfield;
}

/*!
 * @fn void PSStarDome::Create(const char *starfile, float radius, float brightness, const char *moon_texture)
 * @param starfile		the file to read the star data from
 * @param brightness	relative brightness 
 * @param moon_texture	the file containing the moon texture
 *
 * Creates a complete StarDome, including a moon if the moon texture file is included
 *	
 * @see PSStarDome::SetRadius PSStarDome::SetTimeOfDay PSSkyDome::Create PSDayDome::Create
 */
void StarDome::Create(const char *starfile, float brightness, const char *moon_texture)
{
	// Read in the star data
	NumStars = 0;
	RelativeBrightness = brightness;

	Read_Stardata(starfile);


	// Need Appearances to Render GeoMesh!
	Appearance *pApp = new Appearance();
	pApp->Set(Appearance::SHADING, Appearance::POINTS);
	pApp->Set(Appearance::LIGHTING, false);
	SetAppearance(pApp);

	Mesh *geo = new Mesh(GEO_Colors);
	geo->AddVertices(NULL, NumStars);
	AddStars(geo->GetVertices());
	SetGeometry(geo);

	// Set Maximum rotational change dependent only on hours, minutes and secs
	MaxTimeOfDay = TIME_TO_INT(24, 0, 0);

	SetDawnTimes(5, 0, 0, 7, 0, 0);
	SetDuskTimes(17, 0, 0, 19, 0, 0);

	if (moon_texture)
	{
		// Create moon
		Shape* MoonShape = new Shape();
		TriMesh *MoonGeom = new TriMesh(VTX_TexCoords);

		Appearance* app = SkyDome::AddTextureAppearance(moon_texture);
		MoonShape->SetAppearance(app);
		XFGeoUtil::Rect(MoonGeom, 0.1f, 0.1f);
		MoonShape->SetGeometry(MoonGeom);
		MoonShape->SetName("scene.skydome.moon");
		Append(MoonShape);

		// why does Z translation work here?  should be Y translation?!
		Matrix trans;
		trans.Translate(0.0f, 0.0f, 0.9f);
		(*MoonGeom) *= trans;
	}
}

/*!
 * @fn void StarDome::SetRadius(float radius)
 * @param radius	new stardome radius
 *
 * Updates the local matrix to scale the star positions to the given radius.
 *
 * @see SkyDome::SetRadius StarDome::Create
 */
void StarDome::SetRadius(float radius)
{
	Radius = radius;
	Reset();
	Size(radius, radius, radius);
}

/*!
 * @fn void StarDome::SetTimeOfDay(int hr, int min, int sec)
 * @param hr	hour of the day
 * @param min	minute in that hour
 * @param sec	second in that minute
 *
 * Sets time of day, updates star positions and intensity
 *	
 * @see SkyDome::SetTimeOfDay DayDome::SetTimeOfDay
 */
void StarDome::SetTimeOfDay(int hr, int min, int sec)
{
	TimeOfDay = TIME_TO_INT(hr, min, sec); 
	FadeStars();
}

// Helper function to compare stars by magnitude
int	_cdecl compare_mag(const void* star1, const void* star2)
{
	StarDome::Star *pstar1 = (StarDome::Star*) star1;
	StarDome::Star *pstar2 = (StarDome::Star*) star2;
   
	float magdiff = pstar1->mag - pstar2->mag;
	if (magdiff < 0) return -1;
	else if (magdiff > 0) return 1;
	else return 0;
}


/*
 * @fn void PSStarDome::AddStars(PSVtxArray* verts)
 * @name verts	vertex array to which the stars will be added
 *
 * Add stars that have been loaded into the given GeoMesh's vertex array
 *	
 * @see PSStarDome::FadeStars PSStarDome::Create
 */
void StarDome::AddStars(VertexArray* verts)
{
	Vec3	temppt;
	Col4	tempcol, white(1.0f, 1.0f, 1.0f);

	// Add each star to the GeoMesh's vertex array
	for (int i = 0; i < NumStars; i++)
	{
		verts->SetLoc(i, Starfield[i].cartpt);
		tempcol = (Starfield[i].relmag > 1.0f) ? white : white * Starfield[i].relmag; 
#if 0
		TRACE("Star #%d %f RGB(%f,%f,%f)\n", i, Starfield[i].relmag,
			tempcol.r, tempcol.g, tempcol.b);
#endif
		verts->SetColor(i, tempcol);
	}
}


/*
 * @fn void StarDome::FadeStars()
 *
 * Determines how many vertices need to be included in the mesh, based
 * on the time of day and when dawn and dusk are supposed to be
 *	
 * @see PSStarDome::AddStars PSStarDome::SetTimeOfDay
 */
void StarDome::FadeStars()
{
	int numvalidstars = 0;

	Mesh *geo = (Mesh *) GetGeometry();
	if (geo == NULL) return;

	float duskpct, dawnpct;
	float magmod;

	// Set the right transformation for the time of night
	Reset();
	Scale(Radius, Radius, Radius);

	// Set the north star around 20 degrees above the horizon
	Rotate(Vec3(1,0,0), -(0.4f * PI));
	Turn(Vec3(0,1,0), -((float)TimeOfDay/MaxTimeOfDay) * PI * 2.0f);

	duskpct = (float)(TimeOfDay - DuskStartTime)/DuskDuration;
	dawnpct = (float)(DawnEndTime - TimeOfDay)/DawnDuration;
	if (TimeOfDay >= DuskStartTime && TimeOfDay <= DuskEndTime) 
		magmod = duskpct;
	else if (TimeOfDay >= DawnStartTime && TimeOfDay <= DawnEndTime)
		magmod = dawnpct;
	else magmod = 1.00f;

	for (int i = 0; i < NumStars; i++)
	{
		numvalidstars++;
		if (Starfield[i].mag >= (HighMag + MagRange * magmod)) break;
	}
	geo->SetNumVtx(numvalidstars);
}


/*!
 * @fn void StarDome::SetStarFile(const char *starpath)
 * @param starpath	path of the file containing the data
 *
 * Sets the path of the file containing the star data
 *
 * @see StarDome::Create
 */
void StarDome::SetStarFile(const char *starpath)
{
	StarFile = starpath;
}


/*
 * @fn void StarDome::Read_Stardata(const char *starfile)
 * @param starfile	the file from which the star data will be read
 *
 * Loads star information from a file, stores it in an array structure,
 * pre-calculates the relative magnitude percentage of each star 
 *
 * @see StarDome::SetStarFile StarDome::Convert_Starcoord
 */
void StarDome::Read_Stardata(const char *starfile)
{
	int n, numstars = 0, num_file_stars = 0;
	float himag, lomag;
    int ra_h, ra_m, dec_d, dec_m, dec_s;
	float ra_s;

	FILE *starfp = NULL;
	Star *curstar;

	if (!(starfp = fopen(starfile, "r")))
	{
//		TRACE("Couldn't open %s\n", fname);
		return;
	}

	if ((n = fscanf(starfp, "%d %f %f\n", &num_file_stars, 
		              &himag, &lomag)) == EOF)
	{
//		TRACE("Couldn't read number of stars from %s\n", fname);
		return;
	}

	if (!(Starfield = new Star[num_file_stars]))
	{
//		TRACE("Couldn't create space for stars\n");
		return;
	}

	while (numstars++ != num_file_stars)
	{
		curstar =  &Starfield[NumStars]; 
		n = fscanf(starfp, "%d %d %f %d %d %d %f\n", &ra_h, 
			&ra_m, &ra_s, &dec_d, &dec_m, &dec_s, &curstar->mag);

		curstar->ra = (float)ra_h + (float)ra_m/60.0f + (float)ra_s/3600.0f;
		curstar->dec = (float)dec_d + (float)dec_m/60.0f + (float)dec_s/3600.0f;

		if (n == EOF)
		{
//			TRACE("Ran out of stars to read from %s\n", fname);
			return;
		}
		Convert_Starcoord(curstar);

//		if (curstar->mag <= 3.35)	// convenient for finding the big dipper
		if (curstar->mag <= MAX_MAGNITUDE)
			NumStars++;
	}
	lomag = MAX_MAGNITUDE;

//	TRACE("CONFIRM NUM STARS = %d\n", NumStars);
	fclose(starfp);

	// sort the stars by magnitude
	qsort((void *)Starfield, NumStars, sizeof(Star), compare_mag);

	HighMag = himag;
	LowMag = lomag;
	MagRange = fabsf(HighMag) + LowMag;

	// Pre calculate the relative magnitude percentage of each star
	for (int i = 0; i < NumStars; i++)
	{
		Starfield[i].relmag = (MagRange - (Starfield[i].mag + fabsf(HighMag)))
			/(MagRange/RelativeBrightness);
	}
}

/*
 * @fn void PSStarDome::Convert_Starcoord(Star *star)
 * @param star	the star whose coordinates will be converted
 *
 * Converts star coordinate information to x,y,z coordinates
 *
 * @see PSStarDome::Read_Stardata
 */
void StarDome::Convert_Starcoord(Star *star)
{
	Vec3 spherept;

	// Make sure the stars are drawn inside of the sky dome 
	spherept.rho = 0.95f; // calculated for a unit  sphere

	// First off convert from sexagesimal to spherical notation for RA
	// Right Ascension corresponds to theta  Range: 0 - 24 hours
	spherept.theta = star->ra * (TWO_PI/24.0f);

	// Declination corresponds to phi Range: -90 to 90 degrees 
	spherept.phi = DEG_TO_RAD(star->dec);

	PT_SPHERE_TO_CART(spherept, star->cartpt);
}

/*!
 * @fn void PSStarDome::SetDuskTimes(int start_hr, int start_min, int start_sec,
 *	int end_hr, int end_min, int end_sec)
 * @param start_hr		start hour of dusk
 * @param start_min		start minute in that hour
 * @param start_sec		start second in that minute
 * @param end_hr		end hour of the dusk
 * @param end_min		end minute in that hour
 * @param end_sec		end second in that minute
 *
 * @see PSStarDome::SetDawnTimes PSSkyDome::SetDuskTimes PSDayDome::SetDuskTimes
 */
void StarDome::SetDuskTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec)
{
	DuskStartTime = TIME_TO_INT(start_hr, start_min, start_sec);
	DuskEndTime = TIME_TO_INT(end_hr, end_min, end_sec);
	DuskDuration = (float)(DuskEndTime - DuskStartTime);
}

/*!
 * @fn void StarDome::SetDawnTimes(int start_hr, int start_min, int start_sec,
 *	int end_hr, int end_min, int end_sec)
 * @param start_hr		start hour of dawn
 * @param start_min		start minute in that hour
 * @param start_sec		start second in that minute
 * @param end_hr		end hour of the dawn
 * @param end_min		end minute in that hour
 * @param end_sec		end second in that minute
 *
 * @see PSStarDome::SetDuskTimes PSSkyDome::SetDawnTimes
 */
void StarDome::SetDawnTimes(int start_hr, int start_min, int start_sec,
						   int end_hr, int end_min, int end_sec)
{
	DawnStartTime = TIME_TO_INT(start_hr, start_min, start_sec);
	DawnEndTime = TIME_TO_INT(end_hr, end_min, end_sec);
	DawnDuration = (float)(DawnEndTime - DawnStartTime);
}



