/***************************************************************************
                          isotracingoperator.h  -  description
                             -------------------                          */
/**
  The isoTracingOperator class provides the basic routines for tracing a single
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
		duplicated from tracingoperator and specialized for isotropy

       merten | 2009-11-09
		direct implementation of TTI

       merten | 2009-11-06
		dummy support for TTI added

      micheld | 2009-08-19
		all point3D<float> are now given by reference

 ***************************************************************************/


#ifndef ISOTRACINGOPERATOR_H
#define ISOTRACINGOPERATOR_H


#include "include/types.h"
#include "include/consts.h"
#include "include/tracingtypes.h"
#include "utils/linalgorithms.h"
#include "utils/mathutil.h"
#include "ray3d.h"



#include <iostream>

template<class VelModelType>
class isoTracingOperator {

public:
  struct VelModelRep_t
  {

  public:
    VelModelRep_t(VelModelType* _V):V(_V){};
    
  private:
    VelModelRep_t(){}

  private:
    VelModelType* V;

    friend class isoTracingOperator;
  };

public:
    typedef VelModelType VMODEL_T;
    typedef float VREPR_T;

// public methods
public: 
  /// Standard constructor of TracingOperator sets member variables to
  /// impossible values. Init and BindVelModel have to be called to
  /// initialize the integration routines.
  isoTracingOperator();
  ~isoTracingOperator();
  /// Initialize the time step size for the integration scheme.
  /// float _dt is the size of a total step interval.
  /// int _nt is the number of steps this interval is divided into. 
  void Init(const float& _dt);
  /// Trace one ray for one total time step and store old values via ray3D::Store(). 
  void operator()(ray3D& ray, const int ith = 0);
  /// Trace one ray for time step dt. 
  void TraceT(triray3D& ray, const float& _dt, const int ith = 0);
  /// Tracing four rays for one total time step and store old values via ray3D::Store(). 
  void operator () (ray3D* rays[4], const int ith = 0);
  /// Trace one ray for one total time step and store old values via ray3D::Store(). 
  void operator()(triray3D& ray, const int ith = 0);

  /// Bind the Velocity Model to the Tracing Operator 
  void BindModels(const VelModelRep_t& VModels);

  /// Trace ray r for the distance l starting from timestep t and return the total travel time. 
  /** The ray is traced for testing assuming a constant velocity. In a second step the ray
      is traced with a corrected travel-time.*/
  double TraceDistance(ray3D& r, const double l, const int t, const int ith = 0);
  void TraceDistance(kinray3D& r, const double l, const int ith = 0);
  /// Trace 4 rays for the individual distances l starting from timestep t and return the total travel times. 
  /** The rays are traced for testing assuming a constant velocity. In a second step the rays
      are traced with a corrected travel-time.*/
  void TraceDistance(ray3D* ray[4], const float l[4], const int t, float _dt[4], const int ith = 0);
  double TraceToSurface(ray3D& ray, const float finaldepth, const int t, const int ith);
  double TraceToPoint(ray3D& ray, const point3D<float>& DestPoint, const int ith);

  float GetTT(const float dt, const int tstepindex);
  void TraceTstep_RungeKutta_dynamic(const double _dt, ray3D & ray, const int ith);
  /// Trace one ray via Runge-Kutta with time step size _dt.
  void TraceTstep_RungeKutta(const double _dt, kinray3D & ray, const int ith);

  void GetVelocityAt(float& v, const point3D<float>& Point, const int ith = 0);

// public attributes
public: 

// private methods
private:
  void TraceTstep_IsotropicRungeKutta_dynamic(const double _dt, ray3D & ray, const int ith);
  /// Trace four rays via Runge-Kutta with time step size _dt.
  void TraceTstep_RungeKutta(const double _dt, kinray3D* rays[4], const int ith);
  /// Trace four rays via Runge-Kutta with individual time step sizes _dt.
  void TraceTstep_RungeKutta(const double _dt[4], ray3D rays[4], const int ith);
  void TraceTstep_RungeKutta(const double _dt[4], kinray3D rays[4], const int ith);
  /// Perform one time step of one ray with constant velocity field.
  void TraceTstep(const double _dt, ray3D& ray, const double& v, const int ith);
  /// Perform 0th order one time step of one ray with given velocity gradients.
  void TraceTstep(const double _dt, ray3D & ray, const double& v, const point3D<float>& vgrad, const int ith);

  void Eval_at(const point3D<float>& x, const point3D<float>& p, const double Q[3][3], const double P[3][3], 
	       point3D<float>& k_x, point3D<float>& k_p, double k_Q[3][3], double k_P[3][3], const int ith);
  void Eval_Isotrop_at(const point3D<float>& x, const point3D<float>& p, const double Q[3][3], const double P[3][3], 
	       point3D<float>& k_x, point3D<float>& k_p, double k_Q[3][3], double k_P[3][3], const int ith);

  /// Abbreviation for the dynamical ray tracing integration operator actually used
  inline void DRT(const double _dt, ray3D & ray, const int ith)
  {
    TraceTstep_RungeKutta_dynamic(_dt, ray, ith);
  };

// private attributes
private:

  /// Velocity Model that has been binded
  VelModelType * VelModel;

  /// Length of one single step interval in s.
  float dtstep_max;

  
};

//************************
#include "isotracingoperator.cpp"

#endif
