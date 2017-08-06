
#pragma once

#include <vector>
#include <string>
#include "body/Mesh.h"
#include "body/BMatrix.h"

namespace Vixen {
namespace SCAPE {


//desired floating point precision 
#define fpType double

#define NUM_SCAPE_TRIANGLES 25000
#define NUM_SCAPE_VERTICES	12500

//Scape model class.  
//Given a set of pose angles and body shape parameters, this class generates
//a 3d model of the human body with pose dependent deformations

class ScapeModel {
//------------------------------------ Types --------------------------------

public:
	typedef std::vector<Vector3<fpType> > VectorList;
	typedef Vector3<fpType> Vector3fp;
	typedef PMatrix<fpType> PMatrixfp;
	typedef SMatrix<fpType> SMatrixfp;

private:
	//data for each body part
	class BodyPart {
	public:
		void	setJointAngle(const Vector3fp& V);
		int		loadData(const char* name, int startindex = 0);
		void	init(const char* name, int n1, int n2, int n3, int n4);

		SMatrixfp R;					//rotation matrix for this part
		SMatrixfp R2QCoeffs;			//Rotation to Q (pose dependent deformation) coefficients
		int startIndex;					//index of start of body part in the triangle list
		int numTriangles;				//number of triangles in body part
		std::vector<int> neighbors;		//parts connected to this part (in order correlated to r2Qcoeffs).
		std::string id;					//text identifier for reference
	};

//----------------------------------- Members --------------------------------

	VectorList				mTemplateVertices;		//template model vertices
	VectorList				mVertices;				//final model vertices
	VectorList				mEdges1, mEdges2;		//lists of edges per triangle used for all deformations
	Mesh<fpType>			mMesh;					//mesh containing model vertices and triangle list

	int						mMaxShapeVectors;		//maximum number of shape eigenvectors
	int						mNumShapeVectors;		//current number of vectors used
	
	SMatrixfp				mShapeVectors;			//body shape eigenvectors      ( (9*NUM_TRIANGLES)xN )
	SMatrixfp				mBeta;					//body shape coefficients      (  Nx1 )
	SMatrixfp				mD, mMeanD;				//body shape transform matrix  ( (9*NUM_TRIANGLES)x1 or 3x3xNUM_TRIANGLES )

	SMatrixfp				mQ;						//pose dependent transform     (  3x3xNUM_TRIANGLES )
	SMatrixfp				mRtmp;					//temp matrix used for rotations

	std::vector<BodyPart>	mBodyParts;				//list of body parts, their rotations and pose dependent deformations

	std::vector<char>		mVertexFlags;			//flags for vertex fitting (using vector<char> because vector<bool> is not thread safe
	
	//initialize parts tree
	void initBodyParts();

	//generate 2 lists of edges from the triangle list 
	void computeEdges();

	//create list of relative joint angles for given part for use as parameters to compute Q
	void computeRelativeAngles(int npart, SMatrixfp &coeffs);

	//compute pose dependent edge deformations from body part rotations
	void computeQ(const VectorList &Rlist);

	//compute body shape deformations from PCA coefficients
	void computeD(const PMatrixfp &beta);

	//transform edges - edge = RDQ * edge;
	void applyTransformations();

	//recursively fix vertices according to neighbors
	void processVertices(int index, int fromIndex);
	void processVertices2(int index, int fromIndex);
	void addEdge(int index, MeshFace &tri, Vector3fp &v, int j, int &c);

	//generate initial approximation of mesh 
	void approximateMesh();

	//find a set of vertices to best fit new edge values
	void fitMesh();

public:

	//Convert a rotation vector (direction = axis, magnitude = rotation angle) into a 3x3 rotation matrix
	static void rodrigues(const Vector3fp &v, PMatrixfp &Rmatrix);

	//Convert a 3x3 rotation matrix into a rotation vector (direction is axis, magnitude is rotation angle)
	static void invRodrigues(const PMatrixfp &Rmatrix, Vector3fp &v);


	ScapeModel();

	//gets
	int numTriangles() const	{	return NUM_SCAPE_TRIANGLES; };
	int numVertices() const 	{	return NUM_SCAPE_VERTICES; };
	Mesh<fpType> &mesh()		{	return mMesh; };

	//load data required for SCAPE model generation
	void loadScapeData(const char *fname);

	//generate the model given a list of limb rotations and shape coefficients
	void generateModel(VectorList &Rlist, const PMatrixfp &beta);

	void rotateParts();

	//convert list of rotation vectors into rotation matrices and store in each body part
	void setJointAngles(const VectorList &Rlist);

	// return index of body part given it's name
	int		findPartByName(const char* name);

};

}	// end SCAPE
}	// end Vixen