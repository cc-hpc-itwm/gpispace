/***************************************************************************
                          OptionalPrm.h  -  description

     Optional Parameter with boolian flag for set/not set.

                            -------------------
    begin                : Thu May 14 2009
    copyright            : (C) 2009 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef OPTIONALPRM_H
#define OPTIONALPRM_H


/**
  *@author Dirk Merten
  */

template<typename T>
struct OptionalPrm {

// public methods
 public: 
    OptionalPrm():given(false){};
    OptionalPrm& operator = (const T& _v)
	{
	    given = true;
	    value = _v;
	    return *this;
	};
// public attributes
 public: 
    bool given;
    T value;

// privat methods
 private:

// privat attributes
 private:
};

#endif
