/***************************************************************************
                          deg.h  -  description
   
    Alias class for angles given in degree.
    Used to distinguish between degree and rad.
    Initialization with float is allowed.
    Casting to float is forbidden.

                          -------------------
    begin                : Mon Feb 15 2010
    copyright            : (C) 2010 by Dominik Michel
    email                : micheld@itwm.fhg.de
***************************************************************************/


#ifndef DEG_H
#define DEG_H


/**
 *@author Dominik Michel
 */

#include "include/consts.h"

class DEG {

// public methods
 public:
    DEG(){}
    DEG& operator= (const DEG& T)
	{
	    v = T.v;
	    return *this;
	}

/// Constructor with initializaion value of type float 
    DEG(const float& T):v(T){}
    DEG(const double& T):v(T){}

/// Assignment with value of type float 
    DEG& operator= (const float& T)
	{
	    v = T;
	    return *this;
	} 


/// transform to/from radian
    inline float as_rad() { return v * _fPI / 180.0f;  }
    void from_rad(float rad) { v = rad * 180.0f / _fPI;  }

// public attributes
 public:
 
/// Allow for direct access to value and address of value.
/// Deprecated: A getter function will be implemented soon.
    float v;

// private methods
 private:

// private attributes
 private:

};

inline float deg2rad(DEG angle) { return angle.v * _fPI / 180.0f;  }

inline DEG rad2deg(float angle) { return DEG(angle * 180.0f / _fPI); }
inline DEG rad2deg(double angle) { return DEG(angle * 180.0 / _PI); }


#endif
