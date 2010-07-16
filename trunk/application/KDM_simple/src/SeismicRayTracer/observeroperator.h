/***************************************************************************
                          observeroperator.h  -  description
                             -------------------                          */
/**
   The ObserverOperator class provides routines to project the observables
   on the wave front onto the subsurface receivers. 
   *@author Dirk Merten
   */

/*                           ------------------- 
                             begin                : Wed Jan 25 2006
                             copyright            : (C) 2006 by Dirk Merten
                             email                : merten@itwm.fhg.de
***************************************************************************/


#ifndef OBSERVEROPERATOR_H
#define OBSERVEROPERATOR_H


#include "structures/point3d.h"
#include "structures/triangle.h"
#include "receiver.h"
#include "signal.h"

template<class TracingOperator_T> 
class ObserverOperator {
 public: 
  ObserverOperator();
  ObserverOperator(TracingOperator_T* TrOp, const int _ith = 0);
  ~ObserverOperator();
  /// Initialize some geometrical variables belonging to the triangle Tri.
  void Init(const Triangle* Tri);
  /// Save the interpolated observables in the receiver via Receiver::SaveSig.
  void operator()(const Triangle* Tri, Receiver* Recv, const int& _Tstep);
  /// Save the interpolated observables of four receivers via Receiver::SaveSig.
  void operator()(const Triangle* Tri, Receiver* Recv[4], const int& _Tstep);
  /// Generate a signal at the position of Recv at time step _t from the ray tube of Triangle Tri.
  /** Thereto the projection of the receiver position into the 'old' triangle plane along
      the plane normal is calculated, the observables at the 'old' vertices are interpolated
      to this position via barycentric interpolation and this result is traced to the
      real heigth of the receiver above the plane using the TracingOperator */
  RecSig SignIntpol(const Triangle* Tri, Receiver* Recv, const int& _t);
  RecSig SignIntpol_old(const Triangle* Tri, Receiver* Recv, const int& _t);
  /// Generate a signal for four receivers at a time from the same ray tube.
  void SignIntpol(const Triangle* Tri, Receiver* Recv[4], const int& _t, RecSig Signals[4]);
  /// Check, if the receiver is contained in a box defined through Tri.  */
  bool RecvInBox(const Triangle* Tri, const Receiver* Recv);
  /// Check, if the receiver is contained in the ray tube defined through Tri.  
  bool RecvInTube(const Triangle* Tri, const Receiver* Recv);
  kinray3D Interpolate_to_point(const point3D<float>& Rec_pos, const kinray3D& ray0, const kinray3D& ray1, const kinray3D& ray2);
  //inverse distance weighting (IDW) interpolation
  float IDW_Interpol(point3D<float> pX, point3D<float> *P, float *Pval, int nP);
  point3D<float> IDW_Interpol_p3D(point3D<float> pX, point3D<float> *P, point3D<float> *Pval, int nP);

 private: 
  /**  */
  TracingOperator_T * Tracer;

  point3D<float> X0_n;
  point3D<float> X1_n;
  point3D<float> X2_n;

  point3D<float> X0_o;
  point3D<float> X1_o;
  point3D<float> X2_o;

  point3D<float> mid_old;
  point3D<float> mid_new;

  //Normal of old wave face
  point3D<float> n_old;

  // Normal of new wave face
  point3D<float> n_new;

  // kmah index of old face
  int kmah;

  int ith;
};

#include "observeroperator.cpp"

#endif
