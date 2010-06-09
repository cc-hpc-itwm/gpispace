/***************************************************************************
                          memman3d_malloc.h  -  description
                             -------------------
    begin                : Fri Nov 11 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef MEMMAN3D_MALLOC_H
#define MEMMAN3D_MALLOC_H

#include <stdlib.h>
#include <cstddef>
#include <iostream>

/**
  *@author Dirk Merten
  */
template<class d_point>
class MemMan3D_malloc {
public: 
  MemMan3D_malloc();
  ~MemMan3D_malloc();
  /** Constructs and allocates a cube of size Nx*Ny*Nz */
  MemMan3D_malloc(const int& _Nx, const int& _Ny, const int& _Nz);
  /** Return amount of memory needed */
  unsigned long GetMemoryNeeded(const int & _Nx, const int & _Ny, const int & _Nz);
  /** Initializes and allocates a cube of size Nx*Ny*Nz */
  void Init(const int& _Nx, const int& _Ny, const int& _Nz);
  /** Frees all the memory and sets Nx, Ny, Nz to 0 */
  void FreeMem();
  /** Initializes a point at position (x,y,z) with dp */
  void InitPoint(const int& x, const int& y, const int& z, const d_point& dp)
  {
#ifdef DEBUG
  if ( MemVec == NULL)
    std::cerr << "MemMan3D_malloc::InitPoint Memory has not been allocated!\n";
  if ( (x < 0) || (x >= Nx) )
    std::cerr << "MemMan3D_malloc::InitPoint x = " << x << " is out of range!\n";
  if ( (y < 0) || (y >= Ny) )
    std::cerr << "MemMan3D_malloc::InitPoint y = " << y << " is out of range!\n";
  if ( (z < 0) || (z >= Nz) )
    std::cerr << "MemMan3D_malloc::InitPoint z = " << z << " is out of range!\n";
#endif

  MemVec[x][y][z] = dp;
  };
  /** Add dp to the point at position (x,y,z) */
  void AddPoint(const int& x, const int& y, const int& z, const d_point& dp)
  {
#ifdef DEBUG
  if ( MemVec == NULL)
    std::cerr << "MemMan3D_malloc::AddPoint Memory has not been allocated!\n";
  if ( (x < 0) || (x >= Nx) )
    std::cerr << "MemMan3D_malloc::AddPoint x = " << x << " is out of range!\n";
  if ( (y < 0) || (y >= Ny) )
    std::cerr << "MemMan3D_malloc::AddPoint y = " << y << " is out of range!\n";
  if ( (z < 0) || (z >= Nz) )
    std::cerr << "MemMan3D_malloc::AddPoint z = " << z << " is out of range!\n";
#endif

  MemVec[x][y][z] += dp;
  };
  /** Returns reference to the point at position (x,y,z) */
  d_point* GetPointAt(const int& x, const int& y, const int& z) const;

  /** Copy all points to pre-allocated memory at Mem */
  void GetAll(d_point* Mem);
private: // Private attributes
  /** 3-dimensional array of points */
  d_point*** MemVec;
  /** Nubers of point in x-, y- and z-direction */
  int Nx, Ny, Nz;
};

template<class d_point>
MemMan3D_malloc<d_point>::MemMan3D_malloc(){
  MemVec = NULL;
  Nx = 0;
  Ny = 0;
  Nz = 0;
}
template<class d_point>
MemMan3D_malloc<d_point>::~MemMan3D_malloc(){
    FreeMem();
}
/** No descriptions */
template<class d_point>
void MemMan3D_malloc<d_point>::FreeMem(){
  if (MemVec != NULL)
    {
      for (int i = 0; i < Nx; i++)
        {
          for (int j = 0; j < Ny; j++)
            delete[] MemVec[i][j];
	  
          delete[] MemVec[i];
        }
      delete[] MemVec;
    }
  MemVec = NULL;
}

/** No descriptions */
template<class d_point>
MemMan3D_malloc<d_point>::MemMan3D_malloc(const int& _Nx, const int& _Ny, const int& _Nz){
  MemVec = NULL;
  Init(_Nx, _Ny, _Nz);
}

template<class d_point>
unsigned long MemMan3D_malloc<d_point>::GetMemoryNeeded(const int & _Nx, const int & _Ny, const int & _Nz){
  unsigned long MemoryNeeded =  _Nx* sizeof(d_point**) + _Nx*_Ny*sizeof(d_point*) + _Nx*_Ny*_Nz*sizeof(d_point);
  return MemoryNeeded;
}

template<class d_point>
void MemMan3D_malloc<d_point>::Init(const int& _Nx, const int& _Ny, const int& _Nz){
  if (MemVec != NULL)
      FreeMem();

  Nx = _Nx;
  Ny = _Ny;
  Nz = _Nz;

  if ( (Nx < 0) || (Ny < 0) || (Nz < 0))
  {
    std::cerr << "MemMan3D_malloc: Invalid dimensions " 
              << Nx << " " << Ny << " " << Nz << std::endl;
    exit(1);
  }

  MemVec = new d_point**[Nx];
  if (MemVec != NULL)
  {
    for (int i = 0; i < Nx; i++)
    {
      MemVec[i] = new d_point*[Ny];
      if (MemVec != NULL)
      {
        for (int j = 0; j < Ny; j++)
        {
          MemVec[i][j] = new d_point[Nz];//();
          if (MemVec[i][j] != NULL)
            continue;
          else
          {
              std::cerr << "MemMan3D_malloc: Not enough Memory available!\n";
              exit(1);
          }
        }
      }
      else
      {
        std::cerr << "MemMan3D_malloc: Not enough Memory available!\n";
        exit(1);
      }
    }
  }
  else
  {
    std::cerr << "MemMan3D_malloc: Not enough Memory available!\n";
    exit(1);
  }
}
/** No descriptions */
template<class d_point>
d_point* MemMan3D_malloc<d_point>::GetPointAt(const int& x, const int& y, const int& z) const{

#ifdef DEBUG
  if ( MemVec == 0)
    std::cerr << "MemMan3D_malloc::GetPointAt Memory has not been allocated!\n";
  if ( (x < 0) || (x >= Nx) )
    std::cerr << "MemMan3D_malloc::GetPointAt x = " << x << " is out of range!\n";
  if ( (y < 0) || (y >= Ny) )
    std::cerr << "MemMan3D_malloc::GetPointAt y = " << y << " is out of range!\n";
  if ( (z < 0) || (z >= Nz) )
    std::cerr << "MemMan3D_malloc::GetPointAt z = " << z << " is out of range!\n";
#endif

  //std::cout << "MemMan3D_malloc: " << MemVec << " " << x << y << z << std::flush;
  //std::cout << MemVec[x][y][z].grad2[0][0] << std::endl;

  return &MemVec[x][y][z];
}

template<class d_point>
void MemMan3D_malloc<d_point>::GetAll(d_point* Mem)
{
    for (int i = 0; i < Nx; i++)
    {
      for (int j = 0; j < Ny; j++)
      {
        memcpy((void*)(&Mem[(i*Ny + j)*Nz]), MemVec[i][j], Nz*sizeof(d_point));
      }
    }
}

#endif
