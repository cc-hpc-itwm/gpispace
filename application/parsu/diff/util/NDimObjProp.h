//
// C++ Interface: NDimObjProp
//
// Description: 
//
//
// Author: Daniel Gruenewald, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef NDIMOBJPROP_H
#define NDIMOBJPROP_H

/// standard includes
#include <stdlib.h>
#include <iostream>
#include <ostream>
#include <assert.h>

/// includes
#include "pointND.h"

/**
	@author Daniel Gruenewald

      class implements
 */

template <short NDIM, typename DataType>
class NDimObjProp {
  
  public:
    NDimObjProp();
    NDimObjProp( const pointND<NDIM,float> & _X0
        ,const pointND<NDIM,int>   & _N
            ,const pointND<NDIM,float> & _dx
                ,const pointND<NDIM,int>   & _MemLayout);
    ~NDimObjProp();
    
    // Arithmetic operators:
    // Need to be implemented by each of the 
    // derived classes
    
    // addition operator
    virtual NDimObjProp operator+(const NDimObjProp &) const;
    
    // subtraction operator
    virtual NDimObjProp operator-(const NDimObjProp &) const;
    
    // multiplication operator
    virtual NDimObjProp operator*(const NDimObjProp &) const;
    
    // scalar multiplication operator
    virtual NDimObjProp operator*(const DataType &) const;
    
    NDimObjProp & operator=(const NDimObjProp &);
    
    bool operator==(const NDimObjProp &) const;
    
    // Getter routines:
    pointND<NDIM,int>   getN() const;
    
    pointND<NDIM,float> getX0() const;
    
    pointND<NDIM,float> getdx() const;
    
    pointND<NDIM,int> getMemLayout() const;
    
    // GetIndex returns the closest lattice coordinate vector
    // for the given physical vector
    pointND<NDIM,int>   getIndex(const pointND<NDIM,float> &) const;
    
    // GetLowerIndex returns the lower left lattice coordinate
    // vector for the given physical vector
    pointND<NDIM,int>   getLowerIndex(const pointND<NDIM,float> &) const;
    
    // GetCoord returns the physical coordinate vector
    // for the given lattice vector
    pointND<NDIM,float> getCoord(const pointND<NDIM,int>   &) const;
    
    template <short NDIM_OS, typename DataType_OS>
        friend std::ostream& operator<<( std::ostream &,const NDimObjProp<NDIM_OS,DataType_OS> &);
    
  private:
    
    // physical coordinates of the origin
    pointND<NDIM,float> X0;
    
    // lattice extensions
    pointND<NDIM,int> N;
    
    // lattice spacings
    pointND<NDIM,float> dx;
    
    // unit [m,feet]
    pointND<NDIM,int> MemLayout; 
    
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//                                                        //
//             NDimObjProp implementation                 //
//                                                        //
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

template <short NDIM, typename DataType>
    NDimObjProp<NDIM,DataType>::NDimObjProp()
{
  
}

template <short NDIM, typename DataType>
    NDimObjProp<NDIM,DataType>::NDimObjProp( const pointND<NDIM,float> & _X0
    ,const pointND<NDIM,int>   & _N
        ,const pointND<NDIM,float> & _dx
            ,const pointND<NDIM,int>   & _MemLayout)
  : X0(_X0),N(_N),dx(_dx),MemLayout(_MemLayout)
{
  
}

template <short NDIM, typename DataType>
    NDimObjProp<NDIM,DataType>::~NDimObjProp()
{
  
}

 // addition operator
template <short NDIM, typename DataType> 
    NDimObjProp<NDIM,DataType>
        NDimObjProp<NDIM,DataType>::operator+
          (const NDimObjProp & rhs) const
{
  assert( *this == rhs );
      
  NDimObjProp<NDIM,DataType> res(X0,N,dx,MemLayout);
  
  return (res);
}
    
    // subtraction operator
template <short NDIM, typename DataType>    
    NDimObjProp<NDIM,DataType>
        NDimObjProp<NDIM,DataType>::operator-
          (const NDimObjProp & rhs) const
{
  assert( *this == rhs );
      
  NDimObjProp<NDIM,DataType> res(X0,N,dx,MemLayout);
  
  return (res);
}
    
    // multiplication operator
template <short NDIM, typename DataType>
    NDimObjProp<NDIM,DataType>
        NDimObjProp<NDIM,DataType>::operator*
            (const NDimObjProp & rhs) const
{
  assert( *this == rhs );
      
  NDimObjProp<NDIM,DataType> res(X0,N,dx,MemLayout);
  
  return (res);
}
    
    // scalar multiplication operator
template <short NDIM, typename DataType>
    NDimObjProp<NDIM,DataType> 
        NDimObjProp<NDIM,DataType>::operator*
            (const DataType & rhs) const
{
      
  NDimObjProp<NDIM,DataType> res(X0,N,dx,MemLayout);
  
  return (res);
}
    
template <short NDIM, typename DataType>
   NDimObjProp<NDIM,DataType> & NDimObjProp<NDIM,DataType>::operator=(const NDimObjProp<NDIM,DataType> & rhs) 
{
  if( this != &rhs )
  {
    X0 = rhs.X0;
    N  = rhs.N;
    dx = rhs.dx;
    MemLayout = rhs.MemLayout;
  }
  return *this;
}

template <short NDIM, typename DataType>
    bool NDimObjProp<NDIM,DataType>::operator==(const NDimObjProp<NDIM,DataType> & rhs) const
{
  
  if ( !(    X0 == rhs.X0 &&
               N == rhs.N &&
              dx == rhs.dx &&
       MemLayout == rhs.MemLayout ) )
    return false;
  
  return true;
}
    
// Getter routines:
template <short NDIM, typename DataType>
    pointND<NDIM,int>   NDimObjProp<NDIM,DataType>::getN() const
{
  return N;
}
    
template <short NDIM, typename DataType>
    pointND<NDIM,float> NDimObjProp<NDIM,DataType>::getX0() const
{
  return X0;
}
    
template <short NDIM, typename DataType>
    pointND<NDIM,float> NDimObjProp<NDIM,DataType>::getdx() const
{
  return dx;
}

template <short NDIM, typename DataType>
    pointND<NDIM,int> NDimObjProp<NDIM,DataType>::getMemLayout() const
{
  return MemLayout;
}
    
    // GetIndex returns the lattice coordinate vector
    // for the given physical vector
template <short NDIM, typename DataType>
    pointND<NDIM,int>   NDimObjProp<NDIM,DataType>::getIndex(const pointND<NDIM,float> & v_phys) const
{
  pointND<NDIM,int> v_lat;
  
  for(int i=0;i<NDIM;i++)
    v_lat[i] = (int)( (v_phys[i] - X0[i]) / dx[i] + 0.5 );
  
  return v_lat;
}
    
    // GetLowerIndex returns the lower left lattice coordinate
    // vector for the given physical vector
template <short NDIM, typename DataType>
    pointND<NDIM,int>   NDimObjProp<NDIM,DataType>::getLowerIndex(const pointND<NDIM,float> & v_phys) const
{
  pointND<NDIM,int> v_lat;
  
  for(int i=0;i<NDIM;i++)
    v_lat[i] = (int)floorf( (v_phys[i] - X0[i]) / dx[i] );
  
  return v_lat;
}    
    
    // GetCoord returns the physical coordinate vector
    // for the given lattice vector
template <short NDIM, typename DataType>    
    pointND<NDIM,float> NDimObjProp<NDIM,DataType>::getCoord(const pointND<NDIM,int>   & v_lat) const
{
  pointND<NDIM,float> v_phys;
  
  for(int i=0;i<NDIM;i++)
    v_phys[i] = X0[i] + v_lat[i] * dx[i];
  
  return v_phys;
  
}

template <short NDIM, typename DataType>
    std::ostream& operator<<( std::ostream & os,const NDimObjProp<NDIM,DataType> & ObjProp)
{
  os<<"ObjProp:        X0 = "<<ObjProp.X0<<std::endl;
  os<<"ObjProp:         N = "<<ObjProp.N<<std::endl;
  os<<"ObjProp:        dx = "<<ObjProp.dx<<std::endl;
  os<<"ObjProp: MemLayout = "<<ObjProp.MemLayout;
  
  return os;
}

#endif