/***************************************************************************
                          triray3d.h  -  description
                             -------------------
    begin                : Wed Jul 30 2008
    copyright            : (C) 2008 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef TRIRAY3D_H
#define TRIRAY3D_H


/**
  *@author Dirk Merten
  */
#include "structures/point3d.h"
#include "structures/spherical.h"
#include "SeismicRayTracer/kinray3d.h"
#include "SeismicRayTracer/ray3d.h"


class triray3D {
public:
  ~triray3D();
  /** No descriptions */
   triray3D(const point3D<float>& _x, const point3D<float>& _p, const float _v, const Spherical& _SD, const int& _kmah = 0);
  /** No descriptions */
   triray3D(const ray3D& _ray);
  /** No descriptions */
  void Store();
  /** No descriptions */
  void Restore();
public: // Public attributes

   ray3D mainray;
   kinray3D dxray;
   kinray3D dyray;

 private:
  triray3D();

  friend class WFPtSrc;
};


#endif
