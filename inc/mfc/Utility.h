//
//
// functions in Utility.cpp
//
//

Shape *make_skydome(VXAppearances *apps, Col4& horizon_col,
		Col4& azimuth_col, float radius);
void destroy_the_viewed_scene();
void MakeNormals(Model*);
void GetModelDirection(Model *pModel, Vec3 &dir);
void SetModelDirection(Model *pModel, Vec3 dir);

void CreateCameraDialog(Camera *pCamera);

// Radians <-> Degrees Macros, single angle
#define RAD_TO_DEG(x) (x * (180.0f/PI))
#define DEG_TO_RAD(x) (x * (PI/180.0f))
