/***************************************************************************
                          kinray3d.cpp  -  description
                             -------------------
    begin                : Wed Jul 30 2008
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "kinray3d.h"

kinray3D::kinray3D(){
    //A = 0;
  x[0] = 0; x[1] = 0; x[2] = 0;
  p[0] = 0; p[1] = 0; p[2] = 1e-4;
  
  StartDir.phi = 0; StartDir.theta = 0;
}
kinray3D::~kinray3D(){
}
/** No descriptions */
kinray3D::kinray3D(const point3D<float>& _x, const point3D<float>& _p, const float _v, const Spherical& SD){
    //A = 0;
  x = _x;
  p = _p;
  v = _v;

  StartDir = SD;

  Store();
}
/** No descriptions */
void kinray3D::Store(){
  x_old = x;
  p_old = p;
  v_old = v;
}

void kinray3D::Restore(){
  x = x_old;
  p = p_old;
}
