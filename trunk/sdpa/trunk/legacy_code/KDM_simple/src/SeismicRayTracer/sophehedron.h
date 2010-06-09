/***************************************************************************
                          Sophehedron.h  -  description
                             -------------------
    begin                : Fri Dec 2 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef SOPHEHEDRON_H
#define SOPHEHEDRON_H


/**
  *@author Dirk Merten
  */
#include "include/consts.h"
#include "structures/point3d.h"
#include "polygon.h"
#include "hedron.h"
#include "3D-GRT/SopheParam.h"

#include <vector>
#include <math.h>


class SopheHedron: public Hedron {
public: 
  SopheHedron();
  virtual ~SopheHedron();
  /** A SopheHedron with a minimum number of Nmin Verticees is generated. */
  virtual void Init_N(const int& Nmin);
  /** A SopheHedron with a given angle resolution is generated. */
  virtual void Init(const float& dang, const float& beta);
  /** A SopheHedron Cap from a SopheHedron with a minimum number of Nmin Verticees pointing into  direction X with opening angle beta is generated. */
  virtual void Init(const int& Nmin, const point3D<float>& X, const float& beta);
  /** A SopheHedron Cap from a SopheHedron with a minimal angular resolution pointing into  direction X with opening angle beta is generated. */
  virtual void Init(const float& dang, const point3D<float>& X, const float& beta);
  /** Return the first _N_V vertices of the Sophehedron as point in 3D on the sphere in the array _vert. The caller is responsible for the memory.*/
  virtual void GetVertices(point3D<float>* _vert, const int& _N_V);
  /** Return the first _N_F faces of the Sophehedron as Sophegon in 3D in the array _faces. The caller is responsible for the memory.*/
  virtual void GetFaces(polygon* _faces, const int& _N_F);

private: // Private attributes
  /**  */
  std::vector<point3D<float> > Vertices;
   /**  */
  std::vector<polygon> Faces;

};

#endif
