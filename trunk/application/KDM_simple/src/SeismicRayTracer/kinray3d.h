/***************************************************************************
                          kinray3d.h  -  description
                             -------------------
    begin                : Wed Jul 30 2008
    copyright            : (C) 2008 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef KINRAY3D_H
#define KINRAY3D_H


/**
  *@author Dirk Merten
  */
#include "structures/point3d.h"
#include "structures/spherical.h"

class kinray3D {
public:
  ~kinray3D();
  /** No descriptions */
   kinray3D(const point3D<float>& _x, const point3D<float>& _p, const float _v, const Spherical& _SD);
  /** No descriptions */
  void Store();
  /** No descriptions */
  void Restore();

public: // Public attributes
  /** Initital Value: Starting angles at the source */
  Spherical StartDir;
  /** Initital Value: Velocity at the source */
  float v_start;
  point3D<float> x, x_old;
  /**  */
  point3D<float> p, p_old;
  //double A;
  float v, v_old;

 private:
  kinray3D();

  friend class triray3D;
};


#endif
