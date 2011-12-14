/***************************************************************************
                          polyhedron.h  -  description
                             -------------------
    begin                : Fri Dec 2 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

      micheld | 2009-08-07
		adding initialization of cap-generation by angle-parameter
		The reason is that a spherical cap should estimate the area each ray covers, 
		thus determing the number of rays necessary for the whole sphere.
 ***************************************************************************/


#ifndef POLYHEDRON_H
#define POLYHEDRON_H


/**
  *@author Dirk Merten
  */
#include "include/consts.h"
#include "structures/point3d.h"
#include "hedron.h"
#include "polygon.h"
#include "icosahedron.h"
#include "trisplit.h"

#include <vector>
#include <math.h>


class PolyHedron: public Hedron {
public: 
  PolyHedron();
  virtual ~PolyHedron();
  /** A PolyHedron with a minimum number of Nmin Verticees is generated. */
  virtual void Init_N(const int& Nmin);
  /** A PolyHedron with a given angle resolution is generated. */
  virtual void Init(const float& dang, const float& beta);
  /** A PolyHedron Cap from a PolyHedron with a minimum number of Nmin Verticees pointing into  direction X with opening angle beta is generated. */
  virtual void Init(const int& Nmin, const point3D<float>& X, const float& beta);
  /** A PolyHedron Cap from a PolyHedron with a given angle resolution pointing into  direction X with opening angle beta is generated. */
  virtual void Init(const float& dang, const point3D<float>& X, const float& beta);
  /** Return the first _N_V vertices of the polyhedron as point in 3D on the sphere in the array _vert. The caller is responsible for the memory.*/
  virtual void GetVertices(point3D<float>* _vert, const int& _N_V);
  /** Return the first _N_F faces of the polyhedron as polygon in 3D in the array _faces. The caller is responsible for the memory.*/
  virtual void GetFaces(polygon* _faces, const int& _N_F);

private: // Private attributes
  /**  */
  std::vector<point3D<float> > Vertices;
   /**  */
  std::vector<polygon> Faces;

  TriSplit FaceUtils;

private:  
  void Output();
  /** Adds the geometric centroid of the Face as Vertex to the Triangulation */
  void AddPoint(const int&i, const point3D<float>& p);
  int SideIndex(const int& v1, const int& v2, const int& n0);

};

#endif
