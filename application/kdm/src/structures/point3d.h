/***************************************************************************
                          point3d.h  -  description

    3-dimensional vector.

                             -------------------
    begin                : Mon Nov 14 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
***************************************************************************/


#ifndef POINT3D_H
#define POINT3D_H


/**
 *@author Dirk Merten
 */

#include <iostream>
#include <math.h>

template<class ptype = double> 
class point3D {
    
// public methods
  public:
    point3D():x(){
    };
    /** Copy constructor */
    point3D (const point3D& _p)
    {
	x[0] = _p.x[0];
	x[1] = _p.x[1];
	x[2] = _p.x[2];
    };
    /** Initialisation by elements */
    point3D(const ptype & _x1, const ptype & _x2, const ptype & _x3)
    {
	x[0] = _x1;
	x[1] = _x2;
	x[2] = _x3;
    };
    ~point3D(){};
    /** Access to elements via operator [] */
    inline ptype& operator [] (const int& i){return x[i];};
    /** Access to elements via operator [] */
    inline const ptype& operator [] (const int& i) const{return x[i];};
    /** operator = (const point3D& ) */
    point3D& operator = (const point3D& _p)
    {
	x[0] = _p.x[0];
	x[1] = _p.x[1];
	x[2] = _p.x[2];
	return *this;
    };
    /** operator = (point3D& ) */
    point3D& operator = (point3D& _p)
    {
	x[0] = _p.x[0];
	x[1] = _p.x[1];
	x[2] = _p.x[2];
	return *this;
    };
    /** operator + (const point3D& ) */
    inline point3D operator + (const point3D& _p) const
    {return  point3D(x[0]+_p.x[0], x[1]+_p.x[1], x[2]+_p.x[2]);};
    /** operator - (const point3D& ) */
    inline point3D operator - (const point3D& _p) const
    {
	return  point3D(x[0]-_p.x[0], x[1]-_p.x[1], x[2]-_p.x[2]);
    };
    /** operator scalar * (const ptype& ) */
    inline point3D operator * (const ptype& _s) const
    {return point3D(x[0] * _s, x[1] * _s, x[2] * _s);};
    /** operator scalar / (const ptype& ) */
    inline point3D operator / (const ptype& _d) const
    {return point3D(x[0] / _d, x[1] / _d, x[2] / _d);};
    /** operator * (const point3D& ) */
    inline ptype operator * (const point3D& _p) const
    {
	return (x[0]*_p.x[0] + x[1]*_p.x[1] + x[2]*_p.x[2]);
    };
    inline bool operator == (const point3D& _p) const
    {
	return ( (x[0] == _p.x[0]) && (x[1] == _p.x[1]) && (x[2] == _p.x[2]) );
    };

    inline point3D VecProd (const point3D& _p) const
    {
	return point3D(x[1]*_p.x[2]-x[2]*_p.x[1], x[2]*_p.x[0]-x[0]*_p.x[2], x[0]*_p.x[1]-x[1]*_p.x[0]);
    };

    inline ptype norm() const
    {return sqrt(x[0]*x[0]+x[1]*x[1]+x[2]*x[2]);}; 


    /** friend of class point3D
	operator << (ostream& , constpoint3D& ) */
    friend std::ostream& operator<< (std::ostream& os, const point3D& p){
	os << p[0] << " " << p[1] << " " << p[2];
	return os;
    };


// public attributes
  public:

// private methods
  private:

// private attributes
  private:
    ptype x[3];
};

template<class ptype> 
inline point3D<ptype> operator * (const ptype& _s, const point3D<ptype>& _x)
{return _x * _s;};

#endif
