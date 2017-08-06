
#pragma once

#include <vector>
#include "body/SmallVectors.h"

namespace Vixen {

namespace SCAPE {

typedef unsigned long MeshRGB;
typedef Vector3i MeshFace;

//simple triangle object
template<class T>
class MeshTriangle{
public:
	Vector3<T> p1, p2, p3;
	MeshTriangle() {};
	~MeshTriangle() {};
	MeshTriangle(Vector3<T> &v1, Vector3<T> &v2, Vector3<T> &v3) {p1 = v1; p2 = v2; p3 = v3; };
	Vector3<T> Normal() {return ((p3 - p1) * (p2 - p1)); };
};

//Mesh Class
template<class T>
class Mesh{

	std::vector<Vector3<T> > mVertices;							//vertex list
	std::vector<Vector3i > mTriangles;							//triangle list
	//std::vector<MeshRGB> mColors;								//color per vertex
	std::vector<std::vector<int> > mNeighbors;					//list of neighboring vertices for each vertex (indices into mVertices)
	std::vector<std::vector<int> > mTriIndex;					//list of associated triangles for each vertex (indices into mTriangles)

	//make indices start from 0 instead of 1
	void DecrementIndices();

public:
	
	Mesh() {};
	~Mesh() {};

	//Get functions
	inline std::vector<Vector3<T> > &Vertices()			{return mVertices; };
	inline std::vector<Vector3i >  &Triangles()			{return mTriangles; };
	//inline std::vector<MeshRGB > &Colors()				{return mColors; };
	inline std::vector<std::vector<int> > &Neighbors()	{return mNeighbors; };
	inline std::vector<std::vector<int> > &TriIndex()	{return mTriIndex; };

	//get a given triangle from vertex data
	inline MeshTriangle<T> Triangle(int index);

	//calculate the normal to the surface at a given vertex
	Vector3<T> SurfaceNormal(int index);

	//Duplicate mesh
	void CopyFrom(Mesh<T> &m, bool BuildTree);

	//build a list of connected vertices for each vertex
	void BuildNeighbors();

	//build a list of triangles associated with each vertex
	void BuildTriIndex();

	//checks for triangles with invalid vertex indices
	bool CheckValid();

};

//------------------------- Implementation ----------------------------------------------------


//get a given triangle from vertex data
template<class T>
MeshTriangle<T> Mesh<T>::Triangle(int index)
{
	MeshFace &f = mTriangles[index];
	return MeshTriangle<T>(mVertices[f[0]], mVertices[f[1]], mVertices[f[2]]);
}

//calculate the normal to the surface at a given vertex
template<class T>
inline Vector3<T> Mesh<T>::SurfaceNormal(int index)
{
	Vector3<T> n(0, 0, 0);
	if(mTriIndex.size() <= 0)						//if triangle index has not been built, return zero normal
		return n;
	int k = (int)mTriIndex[index].size();
	for(int i = 0; i < k; i++)						//calculate weighted average normal of all triangles containing the vertex
		n += Triangle(mTriIndex[index][i]).Normal();
	n /= (T)k;
	return n;
}

inline void AddVertex(std::vector<int> &list, int p)
{	
	for(int i = 0; i < (int)list.size(); i++)		//add to list if it does not already exist
		if(list[i] == p)
			return;
	list.push_back(p);
}

//build a list of connected vertices for each vertex
template<class T> 
void Mesh<T>::BuildNeighbors()
{	mNeighbors.resize(mVertices.size());
	for(int i = 0; i < (int)mTriangles.size(); i++)
	{	MeshFace f = mTriangles[i];
		AddVertex(mNeighbors[f[0]], f[1]); AddVertex(mNeighbors[f[0]], f[2]);	//create list of connected vertices for each vertex
		AddVertex(mNeighbors[f[1]], f[0]); AddVertex(mNeighbors[f[1]], f[2]);
		AddVertex(mNeighbors[f[2]], f[0]); AddVertex(mNeighbors[f[2]], f[1]);
	}
}

//build a list of triangles per vertex
template<class T> 
void Mesh<T>::BuildTriIndex()
{	mTriIndex.resize(mVertices.size());
	for(int i = 0; i < (int)mTriangles.size(); i++)
	{	mTriIndex[mTriangles[i][0]].push_back(i);
		mTriIndex[mTriangles[i][1]].push_back(i);
		mTriIndex[mTriangles[i][2]].push_back(i);
	}
}

//make indices start from 0 instead of 1
template<class T> 
void Mesh<T>::DecrementIndices()
{	for(int i = 0; i < (int)mTriangles.size(); i++)
	{	mTriangles[i].p1 -= 1;
		mTriangles[i].p2 -= 1;
		mTriangles[i].p3 -= 1;
	}
}


//check for invalid triangle indices
template<class T>
bool Mesh<T>::CheckValid()
{
	int n = (int)mVertices.size();
	for(int i = 0; i < (int)mTriangles.size(); i++)									//check for invalid triangles 
		if(mTriangles[i][0] > n || mTriangles[i][1] > n || mTriangles[i][2] > n ||
		   mTriangles[i][0] < 0 || mTriangles[i][1] < 0 || mTriangles[i][2] < 0)
		{	cout << "Invalid mesh data." << endl;
			return false;
		}
	return true;
}


template<class T>
void Mesh<T>::CopyFrom(Mesh<T> &m, bool buildTree)
{
	mVertices.resize(m.Vertices().size());											//allocate space for data
	mTriangles.resize(m.Triangles().size());
	mTriIndex.resize(m.TriIndex().size());
	mNeighbors.resize(m.Neighbors().size());
	mLandmarks.resize(m.Landmarks().size());

	for(int i = 0; i < (int)mVertices.size(); i++)									//Copy all data from given mesh
	{	mVertices[i] = m.Vertices()[i];
		for(int j = 0; j < (int)m.TriIndex()[i].size(); j++)		
			mTriIndex[i].push_back(m.TriIndex()[i][j]);
		for(int j = 0; j < (int)m.Neighbors()[i].size(); j++)		
			mNeighbors[i].push_back(m.Neighbors()[i][j]);
	}
	
	for(int i = 0; i < (int)mTriangles.size(); i++)									//copy triangle data
		mTriangles[i] = m.Triangles()[i];
	
	for(int i = 0; i < (int)mLandmarks.size(); i++)
		mLandmarks[i] = m.Landmarks()[i];

	if(buildTree)																	//build the K-D tree if desired
		mTree.BuildTree(mVertices);
}


/*
//Load mesh data from ascii vertex and triangle files
template<class T> 
bool Mesh<T>::LoadAscii(const char *vertFile, const char *triFile, bool rewind)
{
	std::cout << vertFile << " : Loading Vertices." << std::endl;
	if(!LoadVector3(vertFile, mVertices, (T)0, false))								//load vertex and triangle data
		return false;
	std::cout << triFile << " : Loading Faces." << std::endl;
	if(!LoadVector3(triFile, mTriangles, (int)0, rewind))
		return false;
	int n = (int)mVertices.size();
	for(int i = 0; i < (int)mTriangles.size(); i++)									//check for invalid triangles 
		if(mTriangles[i].p1 > n || mTriangles[i].p2 > n || mTriangles[i].p3 > n ||
		   mTriangles[i].p1 < 0 || mTriangles[i].p2 < 0 || mTriangles[i].p3 < 0)
		{	cout << "Invalid mesh data loading " << triFile << endl;
			return false;
		}
	DecrementIndices();
	std::cout << "Building K-D tree." << std::endl;
	mTree.BuildTree(mVertices);													//build K-D tree 
	std::cout << "Building index lists." << std::endl;
	BuildNeighbors();
	BuildTriIndex();
	std::cout << "Loaded." << std::endl << std::endl;
	return true;
}

//Save mesh data to ascii vertex and triangle files
template<class T>
void Mesh<T>::SaveAscii(const char *vertFile, const char *triFile)
{
	std::ofstream f(vertFile);
	for(int i = 0; i < (int)mVertices.size(); i++)
	{	Vector3<T> v = mVertices[i];
		f << v.x << " " << v.y << " " << v.z << endl;
	}
	f.close();
	f.open(triFile);
	for(int i = 0; i < (int)mTriangles.size(); i++)
	{	MeshFace &fc = mTriangles[i];
		f << fc.p1 + 1 << " " << fc.p2 + 1<< " " << fc.p3 +1 << endl;
	}
	f.close();
	cout << "Mesh data written to : " << vertFile << " and " << triFile << endl;
}

template<class T>
void Mesh<T>::SaveRaw(char *filename, std::vector<Matrix3x4<T> > &t)
{
	std::ofstream f(filename);
	for(int i = 0; i < (int)mTriangles.size(); i++)
	{	MeshTriangle<T> tri = Triangle(i);
		f << tri.p1.x << " " << tri.p1.y << " " << tri.p1.z << " ";
		f << tri.p2.x << " " << tri.p2.y << " " << tri.p2.z << " ";
		f << tri.p3.x << " " << tri.p3.y << " " << tri.p3.z << std::endl;
	}
	f.close();
}
*/

}	// end SCAPE

}	// end Vixen