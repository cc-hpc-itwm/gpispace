/***************************************************************************
                          ray3d.cpp  -  description
                             -------------------
    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "ray3d.h"

ray3D::ray3D(){
    //A = 0;
  status = false;
  history = 0;

  x[0] = 0; x[1] = 0; x[2] = 0;
  p[0] = 0; p[1] = 0; p[2] = 1e-4;
  
  //e1[0] = 0; e1[1] = 0;
  //e1_half[0] = 0; e1_half[1] = 0;
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      {
        Q[i][j] = 0;
        Q_old[i][j] = 0;
        P[i][j] = 1e-4;
      }
  detQ = 0;
  detQ_old = 0;
  //Q_minor = 0;
  //Q_minor_old = 0;
  kmah = 0;

  StartDir.phi = 0; StartDir.theta = 0;
}
ray3D::~ray3D(){
}
/** No descriptions */
ray3D::ray3D(const point3D<float>& _x, const point3D<float>& _p, const float _v, const Spherical& SD, const int& _kmah){
    //A = 0;
  status = false;
  history = 0;
  x = _x;
  p = _p;
  v = _v;

  //e1[0] = 0; e1[1] = 0;
  //e1_half[0] = 0; e1_half[1] = 0;

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      {
        Q[i][j] = 0;
        Q_old[i][j] = 0;
        P[i][j] = 0;
      }
  detQ = 0;
  detQ_old = 0;
  //Q_minor = 0;
  //Q_minor_old = 0;

  //trace = 0;
  //trace_old = 0;

  kmah = _kmah;

  StartDir = SD;

  Store();
}
/** No descriptions */
void ray3D::Store(){
  x_old = x;
  p_old = p;
  v_old = v;
  detQ_old = detQ;
  //trace_old = trace;
  //Q_minor_old = Q_minor;
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      {
        Q_old[i][j] = Q[i][j];
        P_old[i][j] = P[i][j];
      }
}

void ray3D::Restore(){
  x = x_old;
  p = p_old;
  detQ = detQ_old;
  //trace = trace_old;
  //Q_minor = Q_minor_old;
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      {
        Q[i][j] = Q_old[i][j];
        P[i][j] = P_old[i][j];
      }
}
