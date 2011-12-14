/***************************************************************************
                          memman3d_bucket.h  -  description
                             -------------------
    begin                : Fri Nov 11 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef MEMMAN3D_BUCKET_H
#define MEMMAN3D_BUCKET_H


/**
  *@author Dirk Merten
  */
#include "memman3d_malloc.h"

#include <stdlib.h>
#include <iostream>

/** Defines for the size of the bucket for div and mod as byte shifs */
#define B_SIZE 8
#define B_SIZE_LN_2 3
#define B_SIZE_MOD 7

template<class d_point>
class MemMan3D_bucket {
public: 
  MemMan3D_bucket();
  ~MemMan3D_bucket();
  /** Constructs and allocates a cube of size nBlocksX*nBlocksY*nBlocksZ */
  MemMan3D_bucket(const int& nBlocksX, const int& nBlocksY, const int& nBlocksZ);
  /** Return amount of memory needed */
  unsigned long GetMemoryNeeded(const int & _nBlocksX, const int & _nBlocksY, const int & _nBlocksZ);
 /** Initializes and allocates a cube of size nBlocksX*nBlocksY*nBlocksZ */
  void Init(const int& nBlocksX, const int& nBlocksY, const int& nBlocksZ);
  /** Frees all the memory and sets nBlocksX, nBlocksY, nBlocksZ to 0 */
  void FreeMem();
  /** Initializes a point at position (x,y,z) with dp */
  void InitPoint(const int& x, const int& y, const int& z, const d_point& dp);
  /** Initializes the point with index i */
  int InitPoint(const int & i, const d_point& dp);
  /** Returns reference to the point at position (x,y,z) */
  d_point* GetPointAt(const int& x, const int& y, const int& z) const;
  /** Returns reference to the point with index i */
  d_point & GetPoint(const int & i);
  /** operator [] (const MemMan3D_bucket& ) */
  d_point & operator [] (const int & i );
  /** No descriptions */
  int size();
  int sizexfindgr();
  int sizey();
  int sizez();
private: // Private functions
  int GetIndex(const int& x, const int& y, const int& z) const;
private: // Private attributes
  /** List of memory blocks */
  MemMan3D_malloc<d_point>* MemVec;
  /** Nubers of blocks in x-, y- and z-direction */
  int nBlocksX, nBlocksY, nBlocksZ;
  /** Size of blocks in x-, y- and z-direction */
  int bucket_size_x, bucket_size_y, bucket_size_z;
  /** actual number of elements */
  int _size;
  /** Some Debugging variables for counting the references */
  mutable int refer, referenced;
};

template<class d_point>
MemMan3D_bucket<d_point>::MemMan3D_bucket(){
  MemVec = NULL;
  nBlocksX = 0;
  nBlocksY = 0;
  nBlocksZ = 0;
  bucket_size_x = 0;
  bucket_size_y = 0;
  bucket_size_z = 0;
  _size = 0;
  refer = -1; referenced = 0;
}
template<class d_point>
MemMan3D_bucket<d_point>::~MemMan3D_bucket(){
  FreeMem();
}
template<class d_point>
void MemMan3D_bucket<d_point>::FreeMem(){
  if (MemVec != NULL)
    {
      delete[] MemVec;
      nBlocksX = 0;
      nBlocksY = 0;
      nBlocksZ = 0;
      bucket_size_x = 0;
      bucket_size_y = 0;
      bucket_size_z = 0;
      _size = 0;
    }
  MemVec = NULL;
}
/** No descriptions */
template<class d_point>
MemMan3D_bucket<d_point>::MemMan3D_bucket(const int& _nBlocksX, const int& _nBlocksY, const int& _nBlocksZ){
  Init(_nBlocksX, _nBlocksY, _nBlocksZ);
}

/** No descriptions */
template<class d_point>
unsigned long MemMan3D_bucket<d_point>::GetMemoryNeeded(const int & _nBlocksX, const int & _nBlocksY, const int & _nBlocksZ){
    bucket_size_x = (_nBlocksX > B_SIZE)?B_SIZE:_nBlocksX;
    bucket_size_y = (_nBlocksY > B_SIZE)?B_SIZE:_nBlocksY;
    bucket_size_z = (_nBlocksZ > B_SIZE)?B_SIZE:_nBlocksZ;
    nBlocksX = (_nBlocksX)/bucket_size_x + 1;
    nBlocksY = (_nBlocksY)/bucket_size_y + 1;
    nBlocksZ = (_nBlocksZ)/bucket_size_z + 1;

    unsigned long MemoryNeeded =  nBlocksX*nBlocksY*nBlocksZ * sizeof(MemMan3D_malloc<d_point>);

    MemMan3D_malloc<d_point> TestMem;
    MemoryNeeded +=  nBlocksX*nBlocksY*nBlocksZ * TestMem.GetMemoryNeeded(bucket_size_x,bucket_size_y,bucket_size_z);
    return MemoryNeeded;
}

/** No descriptions */
template<class d_point>
void MemMan3D_bucket<d_point>::Init(const int & _nBlocksX, const int & _nBlocksY, const int & _nBlocksZ){
  bucket_size_x = (_nBlocksX > B_SIZE)?B_SIZE:_nBlocksX;
  bucket_size_y = (_nBlocksY > B_SIZE)?B_SIZE:_nBlocksY;
  bucket_size_z = (_nBlocksZ > B_SIZE)?B_SIZE:_nBlocksZ;
  nBlocksX = (_nBlocksX)/bucket_size_x + 1;
  nBlocksY = (_nBlocksY)/bucket_size_y + 1;
  nBlocksZ = (_nBlocksZ)/bucket_size_z + 1;

  MemVec = new MemMan3D_malloc<d_point>[nBlocksX*nBlocksY*nBlocksZ];
  if (MemVec == NULL)
  {
    std::cerr << "MemMan3D_malloc: Not enough Memory available!\n";
    exit(1);
  }
  for (int i=0; i < nBlocksX*nBlocksY*nBlocksZ; i++)
    MemVec[i].Init(bucket_size_x,bucket_size_y,bucket_size_z);

  _size = 0;

  //std::cout << "Allocted Memory: " << nBlocksX*nBlocksY*nBlocksZ << " * "
  //     <<  bucket_size_x*bucket_size_y*bucket_size_z << " * " << sizeof(d_point) << std::endl;
}
/** No descriptions */
template<class d_point>
void MemMan3D_bucket<d_point>::InitPoint(const int& x, const int& y, const int& z, const d_point& dp){

  _size++;
  const int index = GetIndex(x,y,z);

#ifdef DEBUG
  if ( MemVec == NULL)
    std::cerr << "MemMan3D_bucket: Memory has not been allocated!\n";
#endif

  MemVec[index].InitPoint(x & B_SIZE_MOD, y & B_SIZE_MOD, z & B_SIZE_MOD, dp);
}
/** No descriptions */
template<class d_point>
d_point* MemMan3D_bucket<d_point>::GetPointAt(const int& x, const int& y, const int& z) const{

  const int index = GetIndex(x,y,z);
//  std::cout << index << " ";
//  if ( index == refer)
//    referenced++;
//  else
//  {
//    cout << "Index " << refer << " : " << referenced << " times" << endl;
//    refer = index;
//    referenced = 1;
//  }
#ifdef DEBUG
  if ( MemVec == NULL)
    std::cerr << "MemMan3D_bucket: Memory has not been allocated!\n";
#endif

  //std::cout << "MemMan3D_bucket: " << MemVec[index].GetPointAt(x & B_SIZE_MOD, y & B_SIZE_MOD, z & B_SIZE_MOD).grad2[0][0] << std::endl;

  return MemVec[index].GetPointAt(x & B_SIZE_MOD, y & B_SIZE_MOD, z & B_SIZE_MOD); // == x % (2^B_SIZE_LN_2)
}

template<class d_point>
int MemMan3D_bucket<d_point>::GetIndex(const int& x, const int& y, const int& z) const
{
  const int index = ( ( (x >> B_SIZE_LN_2)*nBlocksY         // == x/(2^B_SIZE_LN_2)
			+ (y >> B_SIZE_LN_2))*nBlocksZ      // == y/(2^B_SIZE_LN_2)
		      + (z >> B_SIZE_LN_2) );               // == z/(2^B_SIZE_LN_2)

#ifdef DEBUG
  if ( (index < 0) || (index >= nBlocksX*nBlocksY*nBlocksZ) )
    std::cerr << "MemMan3D_bucket: index " << index << " out of range!\n";
#endif

  return index;
}

/** Initializes the point with index i */
template<class d_point>
int MemMan3D_bucket<d_point>::InitPoint(const int & i, const d_point& dp){

  int index = i/(bucket_size_x*bucket_size_y*bucket_size_z);
  _size++;

#ifdef DEBUG
  if ( (index < 0) || (index >= nBlocksX*nBlocksY*nBlocksZ) )
    std::cerr << "MemMan3D_bucket: index " << index << " out of range!\n";
#endif

  int i_tmp = i - index* bucket_size_x*bucket_size_y*bucket_size_z;
  int x = i_tmp / (bucket_size_y*bucket_size_z);
  int y = (i_tmp - x * (bucket_size_y*bucket_size_z)) / bucket_size_z;
  int z = i_tmp - x * (bucket_size_y*bucket_size_z) - y * bucket_size_z;
  return MemVec[index].InitPoint(x, y, z, dp);
}
/** Returns reference to the point with index i */
template<class d_point>
d_point& MemMan3D_bucket<d_point>::GetPoint(const int & i){

  int index = i >> (2*B_SIZE_LN_2); // (bucket_size_x*bucket_size_y*bucket_size_z);
//  int index = i/(bucket_size_x*bucket_size_y*bucket_size_z);

#ifdef DEBUG
  if ( (index < 0) || (index >= nBlocksX*nBlocksY*nBlocksZ) )
    std::cerr << "MemMan3D_bucket: index " << index << " out of range!\n";
#endif

  int i_tmp = i & ((B_SIZE_MOD+1)*(B_SIZE_MOD+1)-1);
  int x = i_tmp  >> (B_SIZE_LN_2);
  int y = (i_tmp & (B_SIZE_MOD));
  int z = 0;
  return MemVec[index].GetPointAt(x, y, z);
}
/** operator [] (const MemMan3D_bucket& ) */
template<class d_point>
d_point & MemMan3D_bucket<d_point>::operator [] (const int & i ){
  return GetPoint(i);
}

/** No descriptions */
template<class d_point>
int MemMan3D_bucket<d_point>::size(){
  return _size;
}

#endif
