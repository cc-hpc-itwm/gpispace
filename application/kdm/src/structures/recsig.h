/***************************************************************************
                          recsig.h  -  description

     Recorded signal for kirchhoff migration.

                            -------------------
    begin                : Wed Jan 25 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef RECSIG_H
#define RECSIG_H

#define N_SIGNALS 1 //12
#define INVALID_SIGT -100

#include "structures/spherical.h"
#include "structures/point3d.h"

/**
  *@author Dirk Merten
  */

class RecSig {

// public methods
 public:
    RecSig():arrtime(INVALID_SIGT), Px(0.0f), Py(0.0f), Pz(0.0f), Amp(-1){};
    RecSig(const float& _t, const float& _v,
	   const float& _Px, const float& _Py, const float& _Pz,
	   const float& _A, const Spherical& _StartDir)
      :arrtime(_t),Px(_Px),Py(_Py),Pz(_Pz),v(_v),Amp(_A),StartDir(_StartDir){};
    RecSig(const float& _t, const float& _v,
	   const float& _Px, const float& _Py, const float& _Pz,
	   const float& _dPxdx, const float& _dPydx, const float& _dPzdx,
	   const float& _dPxdy, const float& _dPydy, const float& _dPzdy,
	   const float& _A, const Spherical& _StartDir, const point3D<float> & _StartDirVector)
      :arrtime(_t),Px(_Px),Py(_Py),Pz(_Pz),dPxdx(_dPxdx),dPydx(_dPydx),dPzdx(_dPzdx),dPxdy(_dPxdy),dPydy(_dPydy),dPzdy(_dPzdy),v(_v),Amp(_A),StartDir(_StartDir),StartDirVector(_StartDirVector){};
    ~RecSig(){};

    inline float GetT() const {return arrtime;};
    inline float Getpx() const {return Px;};
    inline float Getpy() const {return Py;};
    inline float Getpz() const {return Pz;};
    inline float Getdpxdx() const {return dPxdx;};
    inline float Getdpydx() const {return dPydx;};
    inline float Getdpzdx() const {return dPzdx;};
    inline float Getdpxdy() const {return dPxdy;};
    inline float Getdpydy() const {return dPydy;};
    inline float Getdpzdy() const {return dPzdy;};
    inline float Getv() const {return v;};
    inline float GetAmp() const {return Amp;};
    inline Spherical GetStartDir() const {return StartDir;};
    inline point3D<float> GetStartDirVector() const {return StartDirVector;};

// public attributes
 public:

// privat methods
 private:

// privat attributes
 private:
    /** Arrival time of the signal */
    float arrtime;
    /** Slowness vector */
    float Px, Py, Pz;
    /** Derivatives ov Slowness vector */
    float dPxdx, dPydx, dPzdx;
    float dPxdy, dPydy, dPzdy;
    /** Velocity */
    float v;
    /** Amplitude of the signal = sqrt(detQ) **/
    float Amp;
    /** Start-off direction of the ray at the source **/
    Spherical StartDir;
    point3D<float> StartDirVector;
};

#endif
