//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//
//	 © 2007, Intel Corporation, licensed under Apache 2.0 
//
//  file : 
//  author : Scott Ettinger - scott.m.ettinger@intel.com
//  description : simple matrix container classes
//				  PMatrix : keeps only pointer to data 
//				  SMatrix : owns its data in memory (derived) 
//				  MatrixList : container for matrices stored
//								linearly in memory
//  modified : 
//--------------------------------------------------------------

#pragma once

#include <vector>
#include <iostream>

namespace Vixen {
namespace SCAPE {

//PMatrix ---- simple matrix container class
template<class T>
class PMatrix {
protected:
	T *mPtr;			//pointer to matrix data
	int mR, mC;			//matrix dimensions

public:
	PMatrix() : mR(0), mC(0), mPtr(NULL) {};									//default constructor
	PMatrix(int r, int c, T *p) : mR(r), mC(c), mPtr(p) {};						//constructor given data
	
	inline virtual void set(int r, int c, T *p) {mR = r; mC = c; mPtr = p; };	//assignment (no data copied)
	inline virtual void set(T *p)				{mPtr = p; };					//set data pointer only
	inline T &operator()(int r, int c)			{return mPtr[r * mC + c]; };	//element access

	inline T *data()				{return mPtr; };
	inline const T *data()  const	{return mPtr; };							//gets
	inline int rows()		const	{return mR; };
	inline int cols()		const	{return mC; };
	inline T *row(int i )			{return &mPtr[mC * i];};

	void print() const															//display contents
	{	for(int i = 0 ; i < mR * mC; i++)
			std::cout << mPtr[i] << (((i + 1) % mC == 0) ? "\n" : " ");
		cout << endl;
	}
	virtual ~PMatrix() {};
};



//SMatrix ---- simple matrix container class with storage (derived from Pmatrix)
template<class T>
class SMatrix : public PMatrix<T> {
protected:
	std::vector<T> mData;		//matrix data storage

public:
	SMatrix() {};
	SMatrix(int r, int c) : PMatrix(r, c, NULL)						//constructor with allocation
	{	mData.resize(r * c); 
		mPtr = &mData[0];
	};
	SMatrix(int r, int c, T *ptr) : PMatrix(r, c, NULL)				//constructor with allocation and deep copy of data
	{	mData.resize(r * c); 
		mPtr = &mData[0];
		set(ptr);
	};	
	inline void resize(int r, int c)								//re-allocate size
	{	mR = r; mC = c;
		mData.resize(r * c); 
		mPtr = &mData[0];
	};				
	void set(T *p)													//copy data from pointer location
	{	memcpy(&mData[0], p, sizeof(T) * mR * mC);
	}
	void set(int r, int c, T *p) {mR = r; mC = c; set(p); };		//set from existing data (deep copy)
};

//MatrixList ---- container for a set of 2d matrices stored linearly in memory
template<class T>
class MatrixList {
	
	T *mPtr;				//pointer to matrix data
	int mR, mC;				//dimensions of 2d matrices in list
	int mSize;				//number of matrices in list
	
public:

	MatrixList(T *ptr, int r, int c, int size) : mPtr(ptr), mR(r), mC(c), mSize(size) {};
	inline T *data()						{return mPtr; };				//gets
	inline const T *data() const			{return mPtr; };
	inline int rows() const					{return mR; };
	inline int cols() const					{return mC; };
	inline int size() const					{return mSize; };

	inline PMatrix<T> operator()(int i)		{return PMatrix<T>(mR, mC, &mPtr[mR * mC * i]); };
};

}	// end SCAPE
}	// end Vixen