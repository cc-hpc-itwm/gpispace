/***************************************************************************
                          spherical.h  -  description

   Spherical coordinate vector.

                             -------------------
    begin                : Tue Jan 31 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef SPHERICAL_H
#define SPHERICAL_H


/**
  *@author Dirk Merten
  */

class Spherical {

// public methods
 public: 
    Spherical();
    Spherical(const double& _phi, const double& _theta);
    ~Spherical();

// public attributes
 public:
    double phi;
    double theta;

// private methods
 private:
    
// private attributes
 private:

};

#endif
