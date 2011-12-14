/***************************************************************************
                          DualTaper.h  -  description
*/

/** Taper for two axis, one is a linear taper (taper.h), the other axis determines the ramp-point

    based on the ramp point f(z) and the width:
      x -> 0 for x less than f(z)-width
           1 for x larger than f(z)+width
	   linear inbetween
    this is done by a taper for [-width,width] where -width->0 and +width->+1
    for this the taper has to be initialized by min=-width, taperwidth=2*width and max=3*width
**/

/*                           -------------------
    begin                : Tue Oct 06 2009
    copyright            : (C) 2009 by Dominik Michel
    email                : micheld@itwm.fhg.de
 ***************************************************************************/


#ifndef DualTAPER_H
#define DualTAPER_H

#include "include/defs.h"
#include "utils/taper.h"
#include <stdlib.h>
#include <math.h>



/**
  *@author Dominik Michel
  */

class DualTaper {

// public methods
 public:
    DualTaper(){};
    ~DualTaper() {};

    /** Create taper and set taper region from _min to _max with taper width on both inner sides **/
    DualTaper(const float _a, const float _b, const float _width);

    /** Activate taper and set taper region from _min to _max with taper width on both inner sides **/
    void Init(const float _a, const float _b, const float _width);
    
    /** Get taper weight for given value val **/
    float operator()(const float z, const float x) const;

    /** Test taper weight for given intervall */
    void test(float z0, float z1, float x0, float x1);


// public attributes
 public:

// private methods
 private:

// private attributes
 private:
    Taper xtaper;

    float a,b; // f(z) = a * z + b
    float width; // taper width around f(z)

    
};

#endif
