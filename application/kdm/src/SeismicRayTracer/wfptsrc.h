/***************************************************************************
                          wfptsrc.h  -  description
                             -------------------
    begin                : Fri Dec 2 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

       merten | 2009-11-11
		support and initialization for general tti included

      micheld | 2009-08-07
		the two initializations with a cone-direction of R have been changed
		Previously having a lower estimation of rays as a parameter,
		they now get an angular radius for the spherical cap each ray should cover, 
		thus calculating the necessary number of rays within the polyhedron-class.
 ***************************************************************************/


#ifndef WFPTSRC_H
#define WFPTSRC_H


/**
  *@author Dirk Merten
  */
#include "structures/vti_velocity.h"
#include "structures/tti_velocity.h"
#include "structures/point3d.h"
#include "structures/triangle.h"
#include "ray3d.h"
#include "hedron.h"
#include "polyhedron.h"
#include "sophehedron.h"

#include "math.h"

class WFPtSrc {

// public methods
public: 
  WFPtSrc();
  ~WFPtSrc();
  /** Create source at x with velocity _v; no rays are prepared */
   WFPtSrc(const point3D<float>& x, const float& _v);
  /** Create source at x with velocity _v, VTI parameters A11, Ex; no rays are prepared */
   WFPtSrc(const point3D<float>& x, const vti_velocity& _vti);
  /** Create source at x with velocity _v, TTI parameters A11, Ex, alpha, beta; no rays are prepared */
   WFPtSrc(const point3D<float>& x, const tti_velocity& _tti);
  /** No descriptions */
   WFPtSrc(const point3D<float>& x, const float& _v, const float& dang, const float& beta, const int Type);
  /** No descriptions */
   WFPtSrc(const point3D<float>& x, const vti_velocity& _vti, const float& dang, const float& beta, const int Type);
  /** No descriptions */
   WFPtSrc(const point3D<float>& x, const tti_velocity& _tti, const float& dang, const float& beta, const int Type);
  /** Create source at x with velocity _v and prepare rays in a cone around R with aperture beta and angle discretization dang */
   WFPtSrc(const point3D<float>& x, const float& _v, const float& dang, const point3D<float>& R, const float& beta, const int Type);
  /** Create source at x with velocity _vti and prepare rays in a cone around R with aperture beta and angle discretization dang */
   WFPtSrc(const point3D<float>& x, const vti_velocity& _vti, const float& dang, const point3D<float>& R, const float& beta, const int Type);
  /** Create source at x with velocity _tti and prepare rays in a cone around R with aperture beta and angle discretization dang */
   WFPtSrc(const point3D<float>& x, const tti_velocity& _tti, const float& dang, const point3D<float>& R, const float& beta, const int Type);
  inline const point3D<float> GetPosition() const
      { return pos;}

  inline float GetVelocity() const
      { return vel_s.v;}

  inline vti_velocity GetVTIVelocity() const
      { return vti_velocity(vel_s.v,vel_s.A11,vel_s.Ex);}

  /** Return the number of rays that have been generated. */
  int GetNoRays() const;
  /** Return all the Rays */
  void GetRays(ray3D* _rays);
  /** Return a copy of the ray with index i. */
  ray3D GetRay(const int & i) const;
  /** Return ray with index i that starts from a translated source to position x with velocity _v. */
  ray3D GetTranslatedRay(const int & i, point3D<float>& new_x, const float& new_v);
  /** Return the number of triangles that are generated. */
  int GetNoTriangles();
  void GetTris(polygon* _tris);
  polygon GetTri(const int& i);
  /** Returns the max width of a 2-dim block, which respects locality. */
  int GetVWidth();
  /** No descriptions */
  int GetFWidth();
  /// Create a ray at the source with starting direction SD.
  ray3D CreateRay(const Spherical& SD);


// public attributes
public: 
// private methods
private:
  /** No descriptions */
  void Set_Slowness(point3D<float>* _V);

// private attributes
private:
  /// Position of the Source
  point3D<float> pos;
  /** Velocity at the source */
  tti_velocity vel_s;

  /**  */
  Hedron* Polyeder;
  /** Number of Rays generated */
  int N_Rays;
  /** */
  ray3D* Rays;
  /** */
  polygon* Tris;
  /** Number of Triangles generated. */
  int N_Tris;
};

#endif
