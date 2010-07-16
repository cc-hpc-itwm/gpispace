/***************************************************************************
                          memman3d_vector.h  -  description
                             -------------------
    begin                : Fri Nov 11 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef MEMMAN3D_VECTOR_H
#define MEMMAN3D_VECTOR_H


/**
  *@author Dirk Merten
  */

template<class disc_point>
class MemMan3D_vector {
public: 
	MemMan3D_vector();
	~MemMan3D_vector();
  /** No descriptions */
  MemMan3D_vector(const int& Nx, const int& Ny, const int& Nz);
  /** No descriptions */
  int InitPoint(const int& x, const int& y, const int& z, const disc_point& dp);
  /** No descriptions */
  disc_point& GetPointAt(const int& x, const int& y, const int& z) const;
private: // Private attributes
  /**  */
  disc_point* MemVec;
  /**  */
  int Nx, Ny, Nz;
};
  
template<class disc_point>
MemMan3D_vector<disc_point>::MemMan3D_vector(){
  MemVec = NULL;
  Nx = 0;
  Ny = 0;
  Nz = 0;
}
template<class disc_point>
MemMan3D_vector<disc_point>::~MemMan3D_vector(){
  delete[] MemVec;
}
/** No descriptions */
template<class disc_point>
MemMan3D_vector<disc_point>::MemMan3D_vector(const int& _Nx, const int& _Ny, const int& _Nz){
  Nx = _Nx;
  Ny = _Ny;
  Nz = _Nz;

  MemVec = new disc_point[Nx*Ny*Nz];
  if (MemVec == NULL)
  {
    std::cerr << "MemMan3D_malloc: Not enough Memory available!\n";
    exit(1);
  }
}
/** No descriptions */
template<class disc_point>
int MemMan3D_vector<disc_point>::InitPoint(const int& x, const int& y, const int& z, const disc_point& dp){
  int index = x*Ny*Nz + y*Nz + z;
  MemVec[index] = dp;
}
/** No descriptions */
template<class disc_point>
disc_point& MemMan3D_vector<disc_point>::GetPointAt(const int& x, const int& y, const int& z) const{
  int index = x*Ny*Nz + y*Nz + z;
  return MemVec[index];
}


#endif
