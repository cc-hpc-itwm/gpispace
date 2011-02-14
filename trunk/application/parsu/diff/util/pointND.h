/***************************************************************************
                          pointND.h  -  description

    N-dimensional vector.

                             -------------------
    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Daniel Grünewald
    email                : daniel.gruenewald@itwm.fhg.de
***************************************************************************/


#ifndef POINTND_H
#define POINTND_H


/**
 *@author Daniel Grünewald
 */

#include <iostream>
#include <math.h>
#include <stdexcept>

template<short NDIM = 3, typename ptype = double> 
class pointND {
    
// public methods
  public:
    
    pointND():x()
    {
      
    };
    
    /** Copy constructor */
    pointND (const pointND& _p)
    {
        for(int i = 0; i < NDIM; i++)
          x[i] = _p.x[i];
    };
    
    /** Initialisation by elements */
    pointND(const ptype _x[NDIM])
    {
      for(int i = 0; i < NDIM; i++)
        x[i] = _x[i];
    };
    
    pointND(const ptype & _x)
    {
      if (NDIM != 1)
      {
	throw std::runtime_error("called pointND (x)u for NDIM !=1");
      }
      x[0] = _x;
    }
    
    ~pointND(){};
    
    /** Access to elements via operator [] */
    inline ptype& operator [] (const int& i)
    {
      return x[i];
    };
    
    /** Access to elements via operator [] */
    inline const ptype& operator [] (const int& i) const
    {
      return x[i];
    };
    
    /** operator = (const pointND& ) */
    pointND& operator = (const pointND& _p)
    {
        if( this != &_p)
        {
          for(int i = 0; i < NDIM; i++)
            x[i] = _p.x[i];
        }
	return *this;
    };
    
    /** operator = (pointND& ) */
    pointND& operator = (pointND& _p)
    {
        if ( this != &_p)
        {
          for(int i = 0; i< NDIM; i++)
          {
            x[i] = _p.x[i];
          }
        }
	return *this;
    };
    
    /** operator + (const pointND& ) */
    inline pointND operator + (const pointND& _p) const
    {
        ptype res[NDIM];
        
        for(int i = 0; i < NDIM; i++) 
          res[i] = x[i] + _p.x[i];
        
        return  pointND(res);
    };
    
    /** operator - (const pointND& ) */
    inline pointND operator - (const pointND& _p) const
    {
        ptype res[NDIM];
        
        for(int i = 0; i < NDIM; i++)
          res[i] = x[i] - _p.x[i];
        
	return  pointND(res);
    };
    
    /** operator scalar * (const ptype& ) */
    inline pointND operator * (const ptype& _s) const
    {
        ptype res[NDIM];
        
        for(int i = 0; i < NDIM; i++)
          res[i] = x[i] * _s;
        
        return pointND(res);
    };
    
    /** operator scalar / (const ptype& ) */
    inline pointND operator / (const ptype& _d) const
    {
        ptype res[NDIM];
        
        for(int i = 0; i < NDIM; i++)
          res[i] = x[i] / _d;
        
        return pointND(res);
    };
    
    /** operator * (const pointND& ) */
    inline ptype operator * (const pointND& _p) const
    {
        ptype sqrsum = x[0] * _p.x[0];
        for(int i = 1; i < NDIM; i++)
          sqrsum += x[i] * _p.x[i];
        
	return sqrsum;
    };
    
    inline bool operator == (const pointND& _p) const
    {
      
        for(int i = 0; i < NDIM; i++)
        {
          if( x[i] != _p.x[i])
            return false;
        }
        
	return true;
        
    };

    inline ptype norm() const
    {
      
      ptype sqrsum = x[0] * x[0];
      for(int i = 1; i < NDIM; i++)
        sqrsum += x[i] * x[i];
      
      return sqrt(sqrsum);
      
    }; 


    /** friend of class pointND
	operator << (ostream& , constpointND& ) */
    friend std::ostream& operator<< (std::ostream& os, const pointND& p)
    {
        os<<"( ";
        for(int i = 0; i < NDIM - 1; i++)
          os<<p[i]<<" , ";
	os<<p[NDIM - 1]<<" )";
        
	return os;
    };

    
    /// Specializations
//     inline pointND VecProd (const pointND& _p) const
//     {
//       return pointND(x[1]*_p.x[2]-x[2]*_p.x[1], x[2]*_p.x[0]-x[0]*_p.x[2], x[0]*_p.x[1]-x[1]*_p.x[0]);
//     };

// public attributes
  public:

// private methods
  private:

// private attributes
  private:
    ptype x[NDIM];
};

template<short NDIM , typename ptype> 
inline pointND<NDIM,ptype> operator * ( const ptype& _s
                                       ,const pointND<NDIM,ptype>& _x)
{
  return _x * _s;
}

#endif
