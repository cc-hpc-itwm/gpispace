/***************************************************************************
                          taper.h  -  description
*/

/** Linear Taper
                   x <  min          : 0
            min <= x <  min + width  : linear ramp
    min + width <= x <= max - width  : 1
    max - width <  x <= max          : linear ramp
            max <  x                 : 0

**/

/*                           -------------------
    begin                : Thu Oct 18 2007
    copyright            : (C) 2007 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef TAPER_H
#define TAPER_H

#include "include/defs.h"

#include <stdlib.h>



/**
  *@author Dirk Merten
  */

class Taper {

// public methods
 public:
    Taper():taper_array(NULL),notaper(true){};
    ~Taper();

    /** Create taper and set taper region from _min to _max with taper width on both inner sides **/
    Taper(const float _min, const float _max, const float width);

    /** Activate taper and set taper region from _min to _max with taper width on both inner sides **/
    void Init(const float _min, const float _max, const float width);
    
    /** Get taper weight for given value val **/
    float operator()(const float val) const;

// public attributes
 public:

// private methods
 private:

// private attributes
 private:
    float* taper_array;
    // Dimension array
    int Narray;
    // Minimal and maximal float value
    float fmin, fmax;
    // Taper width
    float width;
    // Switch on or of
    bool notaper;

    
};

#endif
