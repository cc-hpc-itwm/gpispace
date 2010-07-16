/***************************************************************************
                          mod.h  -  description
   
    Alias class for model coordinates.
    Used to distinguish different cordinate frames.
    Initialization with float is allowed.
    Casting to float is forbidden.

                          -------------------
    begin                : Mon Jun 29 2009
    copyright            : (C) 2009 by Dirk Merten
    email                : merten@itwm.fhg.de
***************************************************************************/


#ifndef MOD_H
#define MOD_H


/**
 *@author Dirk Merten
 */

#include <math.h>

class MOD {

// public methods
 public:
    MOD(){}
    MOD& operator= (const MOD& T)
	{
	    v = T.v;
	    return *this;
	}

/// Constructor with initializaion value of type float 
    MOD(const float& T):v(T){}

/// Assignment with value of type float 
    MOD& operator= (const float& T)
	{
	    v = T;
	    return *this;
	} 

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

MOD operator +(const MOD& A, const MOD& B);

MOD operator -(const MOD& A, const MOD& B);

MOD operator *(const MOD& A, const MOD& B);

MOD operator /(const MOD& A, const MOD& B);

float fabsf(const MOD& T);

float sqrt(const MOD& T);

#endif
