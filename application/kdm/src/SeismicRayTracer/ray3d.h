/***************************************************************************
                          ray3d.h  -  description
                             -------------------
    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef RAY3D_H
#define RAY3D_H


/**
  *@author Dirk Merten
  */
#include "structures/point3d.h"
#include "structures/spherical.h"

class ray3D {

// public methods
public:
  ~ray3D();
  /** No descriptions */
   ray3D(const point3D<float>& _x, const point3D<float>& _p, const float _v, const Spherical& _SD, const int& _kmah = 0);
   /** Save current observables */
   void Store();
   /** Restore last observables */
   void Restore();

 // public attributes
 public:
   /** Initital Value: Starting angles at the source */
   Spherical StartDir;
   /** Initital Value: Velocity at the source */
   float v_start;
   point3D<float> x, x_old;
   /**  */
   point3D<float> p, p_old;
   //double A;
   float v, v_old;
   /**  */
   bool status;
   /**  */
   int history;
   /** Dynamic ray tracing variables*/
   float Q[3][3];
   float Q_old[3][3];
   float P[3][3];
   float P_old[3][3];
   /** Determinant of Q for 2 successing time steps */
   float detQ, detQ_old;
   /// kmah index
   int kmah;

// private methods
 private:
  ray3D();

// private attributes
 private:


  friend class WFPtSrc;
  friend class triray3D;
  friend class TracingOperatorTest;
};


#endif
