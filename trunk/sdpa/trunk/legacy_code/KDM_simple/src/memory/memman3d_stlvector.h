/***************************************************************************
                          memman3d_stlvector.h  -  description
                             -------------------
    begin                : Fri Nov 11 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef MEMMAN3D_STLVECTOR_H
#define MEMMAN3D_STLVECTOR_H


/**
  *@author Dirk Merten
  */
#include<vector>

template<class disc_point>
class MemMan3D_stlvector {
public: 
  MemMan3D_stlvector();
  ~MemMan3D_stlvector();
  /** No descriptions */
  MemMan3D_stlvector(const int& Nx, const int& Ny, const int& Nz);
  /** No descriptions */
  void Init(const int& Nx, const int& Ny, const int& Nz);
  /** No descriptions */
  int InitPoint(const int& x, const int& y, const int& z, const disc_point& dp);
  /** No descriptions */
  disc_point& GetPointAt(const int& x, const int& y, const int& z) const;
private: // Private attributes
  /**  */
  std::vector<disc_point> MemVec;
  /**  */
  int Nx, Ny, Nz;
};
  
template<class disc_point>
MemMan3D_stlvector<disc_point>::MemMan3D_stlvector(){
  MemVec.clear();
  Nx = 0;
  Ny = 0;
  Nz = 0;
}                             
template<class disc_point>
MemMan3D_stlvector<disc_point>::~MemMan3D_stlvector(){
}
/** No descriptions */
template<class disc_point>
MemMan3D_stlvector<disc_point>::MemMan3D_stlvector(const int& _Nx, const int& _Ny, const int& _Nz){
  Init(_Nx, _Ny, _Nz);
}
/** No descriptions */
template<class disc_point>
void MemMan3D_stlvector<disc_point>::Init(const int& _Nx, const int& _Ny, const int& _Nz){
  Nx = _Nx;
  Ny = _Ny;
  Nz = _Nz;

  MemVec.reserve(Nx*Ny*Nz);
}
/** No descriptions */
template<class disc_point>
int MemMan3D_stlvector<disc_point>::InitPoint(const int& x, const int& y, const int& z, const disc_point& dp){
  int index = x*Ny*Nz + y*Nz + z;
  MemVec[index] = dp;
}
/** No descriptions */
template<class disc_point>
disc_point& MemMan3D_stlvector<disc_point>::GetPointAt(const int& x, const int& y, const int& z) const{
  int index = x*Ny*Nz + y*Nz + z;
  return (disc_point&) MemVec[index];
}


#endif
