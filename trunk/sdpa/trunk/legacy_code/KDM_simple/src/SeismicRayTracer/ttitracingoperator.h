/***************************************************************************
                          ttitracingoperator.h  -  description
                             -------------------                          */
/**
  The ttiTracingOperator class provides the basic routines for tracing a single
  ray or a bundle of rays for a preset number of time steps or a given 
  distance in space. Several time integration routines can be used, but the
  default 'working horse' is a 4th order Runge-Kutta scheme.
  The total time step can be divided into several (_nt) single steps to
  increase the accuracy of the integration routine in an independent way.
  All the necessary information about the velocity model, velocity,
  velocity gradient, etc. are obtained from the PropModel member variable,
  that is bound to the velocity model in the initialization phase.
  This class encapsulated all the routines that update the rays in the main
  time loop.
  *@author Dirk Merten
  */

/*                           -------------------
    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

       merten | 2010-18-01
		duplicated from tracingoperator and specialized for tti anisotropy

       merten | 2009-11-09
		direct implementation of TTI

       merten | 2009-11-06
		dummy support for TTI added

      micheld | 2009-08-19
		all point3D<float> are now given by reference

 ***************************************************************************/


#ifndef TTITRACINGOPERATOR_H
#define TTITRACINGOPERATOR_H


#include "include/types.h"
#include "include/consts.h"
#include "include/tracingtypes.h"
#include "structures/tti_velocity.h"
#include "utils/linalgorithms.h"
#include "utils/mathutil.h"
#include "utils/propmodel.h"
#include "ray3d.h"



#include <iostream>

template<class VelModelType>
class ttiTracingOperator {

public:
  struct VelModelRep_t
  {

  public:
    VelModelRep_t(VelModelType* _V,
		  VelModelType* _Vh,
		  VelModelType* _Ex,
		  PropModel<VelGridPointLinear>* _Alpha,
		  PropModel<VelGridPointLinear>* _Beta):V(_V),Vh(_Vh),Ex(_Ex),Alpha(_Alpha),Beta(_Beta){};
    
  private:
    VelModelRep_t(){}

  private:
    VelModelType* V;
    VelModelType* Vh;
    VelModelType* Ex;
    PropModel<VelGridPointLinear>* Alpha;
    PropModel<VelGridPointLinear>* Beta;

    friend class ttiTracingOperator;
  };

public:
    typedef VelModelType VMODEL_T;
    typedef tti_velocity VREPR_T;

// public methods
public: 
  /// Standard constructor of TracingOperator sets member variables to
  /// impossible values. Init and BindVelModel have to be called to
  /// initialize the integration routines.
  ttiTracingOperator();
  ~ttiTracingOperator();
  /// Initialize the time step size for the integration scheme.
  /// float _dt is the size of a total step interval.
  /// int _nt is the number of steps this interval is divided into. 
  void Init(const float& _dt);
  /// Trace one ray for one total time step and store old values via ray3D::Store(). 
  void operator()(ray3D& ray, const int ith = 0);
  /// Tracing four rays for one total time step and store old values via ray3D::Store(). 
  void operator () (ray3D* rays[4], const int ith = 0);

  /// Bind the Velocity Models to the Tracing Operator 
  void BindModels(const VelModelRep_t& VModels);

  double TraceToSurface(ray3D& ray, const float finaldepth, const int t, const int ith);
  double TraceToPoint(ray3D& ray, const point3D<float>& DestPoint, const int ith);

  float GetTT(const float dt, const int tstepindex);
  void TraceTstep_TTIRungeKutta_dynamic(const double _dt, ray3D & ray, const int ith);

  void GetVelocityAt(tti_velocity& tti, const point3D<float>& Point, const int ith = 0);

// public attributes
public: 

// private methods
private:
  void Eval_at(const point3D<float>& x, const point3D<float>& p, const double Q[3][3], const double P[3][3], 
	       point3D<float>& k_x, point3D<float>& k_p, double k_Q[3][3], double k_P[3][3], const int ith);

  /// Abbreviation for the dynamical ray tracing integration operator actually used
  inline void DRT(const double _dt, ray3D & ray, const int ith)
  {
    TraceTstep_TTIRungeKutta_dynamic(_dt, ray, ith);
  };


// private attributes
private:

  /// Velocity Model that has been binded
  VelModelType * VelModel;

  /// VTI Property Models that have been binded in VTI/TTI Mode
  VelModelType * VhModel;
  VelModelType * ExModel;


  /// TTI Property Models that have been binded in TTI Mode
  PropModel<VelGridPointLinear>* TTIAlphaModel;
  PropModel<VelGridPointLinear>* TTIBetaModel;

  /// Length of one single step interval in s.
  float dtstep_max;

  
};

//************************
#include "ttitracingoperator.cpp"

#endif
