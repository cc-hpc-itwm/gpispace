/***************************************************************************
                          utm.h  -  description
   
    Alias class for UTM coordinates.
    Used to distinguish different cordinate frames.
    Initialization with float is allowed.
    Casting to float is forbidden.

                          -------------------
    begin                : Mon Jun 29 2009
    copyright            : (C) 2009 by Dirk Merten
    email                : merten@itwm.fhg.de
***************************************************************************/


#ifndef UTM_H
#define UTM_H


/**
 *@author Dirk Merten
 */

class UTM {

// public methods
 public:
    UTM() : v(0.0) {}
    UTM(const float& T) : v(T) {}

/// Constructor with initializaion value of type float 
    const UTM& operator= (const UTM& T)
	{
	    v = T.v;
	    return *this;
	}

/// Assignment with value of type float 
    const UTM& operator= (const float& T)
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

UTM operator +(const UTM& A, const UTM& B);

UTM operator -(const UTM& A, const UTM& B);

UTM operator *(const UTM& A, const UTM& B);

UTM operator /(const UTM& A, const UTM& B);

float fabsf(const UTM& T);

float sqrt(const UTM& T);

#endif
