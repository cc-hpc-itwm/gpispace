/***************************************************************************
                          Hedron.h  -  description
                             -------------------
    begin                : Fri Dec 2 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef HEDRON_H
#define HEDRON_H


/**
  *@author Dirk Merten
  */
#include "include/consts.h"
#include "structures/point3d.h"
#include "polygon.h"

#include <vector>
#include <math.h>


class Hedron {
public: 
  virtual ~Hedron(){};
  /** A Hedron with a minimum number of Nmin Verticees is generated. */
  virtual void Init_N(const int& Nmin) = 0;
  /** A Hedron with a given angle resolution is generated. */
  virtual void Init(const float& dang, const float& beta) = 0;
  /** A Hedron Cap from a Hedron with a minimum number of Nmin Verticees pointing into  direction X with opening angle beta is generated. */
  virtual void Init(const int& Nmin, const point3D<float>& X, const float& beta) = 0;
  virtual void Init(const float& dang, const point3D<float>& X, const float& beta) = 0;
  /** Return the first _N_V vertices of the hedron as point in 3D on the sphere in the array _vert. The caller is responsible for the memory.*/
  virtual void GetVertices(point3D<float>* _vert, const int& _N_V) = 0;
  /** Return the first _N_F faces of the hedron as polygon in 3D in the array _faces. The caller is responsible for the memory.*/
  virtual void GetFaces(polygon* _faces, const int& _N_F) = 0;

  /** Actual number of vertices N_V and number of faces N_F. */
  unsigned int N_V, N_F;
  /** Number of vertices or faces that are "close to each other" in some sense. This number can be used to optimize memory performance. */
  int width;

private: // Private attributes

};

#endif
