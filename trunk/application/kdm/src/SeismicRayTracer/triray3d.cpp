/***************************************************************************
                          triray3d.cpp  -  description
                             -------------------
    begin                : Wed Jul 30 2008
    copyright            : (C) 2008 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "triray3d.h"

triray3D::triray3D(){
}
triray3D::~triray3D(){
}
/** No descriptions */
triray3D::triray3D(const point3D<float>& _x, const point3D<float>& _p, const float _v, const Spherical& SD, const int& _kmah)
    :mainray(_x, _p, _v, SD, _kmah),dxray(point3D<float>(_x[0]+10.0,_x[1],_x[2]),_p,_v,SD),dyray(point3D<float>(_x[0],_x[1]+10.0,_x[2]),_p,_v,SD)
{
}
/** No descriptions */
triray3D::triray3D(const ray3D& _ray)
:mainray(_ray),dxray(point3D<float>(_ray.x[0]+10.0,_ray.x[1],_ray.x[2]),_ray.p,_ray.v,_ray.StartDir),dyray(point3D<float>(_ray.x[0],_ray.x[1]+10.0,_ray.x[2]),_ray.p,_ray.v,_ray.StartDir)
{
    
}
/** No descriptions */
void triray3D::Store(){
  mainray.Store();
  dxray.Store();
  dyray.Store();
}

void triray3D::Restore(){
    mainray.Restore();
    dxray.Restore();
    dyray.Restore();
}
