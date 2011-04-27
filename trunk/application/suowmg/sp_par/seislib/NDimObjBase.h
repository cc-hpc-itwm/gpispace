//
// C++ Interface: NDimObjBase
//
// Description: 
//
//
// Author: Daniel Gruenewald, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef NDIMOBJBASE_H
#define NDIMOBJBASE_H

/// standard includes
#include <stdlib.h>
#include <iostream>
#include <ostream>
#include <assert.h>

/// includes
#include "NDimDataBase.h"
#include "NDimObjProp.h"

/**
	@author Daniel Gruenewald

      class implements an N-dimensional Object:
	    A N-dimensional object is a N-dimensional Data Object
	    supplemented by a object property. 
 */

template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
    class NDimObjBase : public NDimDataBase< NDIM
				            ,DataType
				            ,MetaInfoClass > 
{
       
  public:
    
    // standard constructor
    NDimObjBase(const char * pObjData, const ObjPropClass & ObjProp);
    NDimObjBase(const NDimDataBase<NDIM,DataType,MetaInfoClass> &, const ObjPropClass &);
    // destructor
    ~NDimObjBase();
        
//     // pointwise addition
//     NDimObjBase operator+(const NDimObjBase &) const;
//     // pointwise subtraction
//     NDimObjBase operator-(const NDimObjBase &) const;
//     // pointwise multiplication
//     NDimObjBase operator*(const NDimObjBase &) const;
//     // scalar multiplication
//     NDimObjBase operator*(const DataType &) const;
    
    // copy constructor
    NDimObjBase(const NDimObjBase &);
    // assignment operator
    NDimObjBase & operator=(const NDimObjBase &);
    
    // equality operator
    bool operator==(const NDimObjBase &) const;
    
    // reordering operator
    // changes the directions
    // index i = {0,1,2} denotes the slowest direction
    //                   of the transposed object
    // index j = {0,1,2} denotes the intermediate direction
    //                   of the transposed object
    // index k = {0,1,2} denotes the fastest direction 
    //                   of the transposed object
    // i,j,k have to be 
    NDimObjBase reorder(const pointND<NDIM,int> & Perm);
    
    // getSubMem returns a submemory region
    // of the given object extended from the
    // lower left corner ll to the upper right
    // corner ur
    NDimObjBase getSubObj(pointND<NDIM,float> ll, pointND<NDIM,float> ur);
    
    // setSubMem sets a submemory region
    // of the given object with the values
    // given by the NDimMem object beginning
    // from the lower left corner ll of the 
    // given object
    bool  setSubObj(pointND<NDIM,float> ll, NDimObjBase &);
        
    template <short NDIM_OS, typename DataType_OS, typename MetaInfoClass_OS, typename ObjPropClass_OS>
        friend std::ostream& operator<<
            ( std::ostream &,NDimObjBase<NDIM_OS,DataType_OS,MetaInfoClass_OS,ObjPropClass_OS> & );
            
  protected:
    
    NDimObjBase();
    
    
  protected:
    
    ObjPropClass mObjProp;
    
};
    

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//                                                        //
//             NDimObjBase implementation                     //
//                                                        //
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
    NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass>::NDimObjBase
        ( const char * _pObjData, const ObjPropClass & _ObjProp)
  : NDimDataBase<NDIM,DataType,MetaInfoClass>::NDimDataBase(_pObjData,_ObjProp.getN())
   ,mObjProp(_ObjProp)
{
  
}
 
template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
    NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass>::NDimObjBase
        (const NDimDataBase<NDIM,DataType,MetaInfoClass> & _NDimDataBase, const ObjPropClass & _ObjProp)
  : NDimDataBase<NDIM,DataType,MetaInfoClass>::NDimDataBase(_NDimDataBase)
   ,mObjProp(_ObjProp)
{
  
}
 
template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
    NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass>::~NDimObjBase()
{
  
}

// // pointwise addition
// template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
//     NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass> 
//         NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass>::operator+
//             (const NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass> & rhs) const
// {
//   NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass> res( NDimData<NDIM,DataType,MetaInfoClass>::operator+(rhs)
//                                            ,mObjProp + rhs.mObjProp);
// 
//   return res;
// }
//     
//     // pointwise subtraction
// template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
//     NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass> 
//         NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass>::operator-
//             (const NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass> & rhs) const
// {
//   
// }
// 
//     // pointwise multiplication
// template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
//     NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass> 
//         NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass>::operator*
//             (const NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass> & rhs) const
// {
//   
// }
// 
// // pointwise scalar multiplication
// template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
//     NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass> 
//         NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass>::operator*
//             (const DataType & rhs) const
// {
//   
// }

template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
  NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass>::NDimObjBase
    (const NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass> & _NDimObjBase)
{
  operator=(_NDimObjBase);
}

// assignment operator
template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
    NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass> & 
        NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass>::operator=
            (const NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass> & rhs)
{
  #ifdef DEBUG
  std::cout<<"NDimObjBase<"<<NDIM<<","<<typeid(DataType).name()
  <<","<<typeid(MetaInfoClass).name()<<">::operator="<<std::endl;
  #endif
  if( this != &rhs )
  {
    NDimDataBase<NDIM,DataType,MetaInfoClass>::operator=(rhs);
    mObjProp = rhs.mObjProp;
  }
  
  return *this;
}
    
// equality operator
template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
    bool 
        NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass>::operator==
        (const NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass> & rhs) const
{
  return ( ( NDimDataBase<NDIM,DataType,MetaInfoClass>::operator==(rhs) ) && 
           ( mObjProp == rhs.mObjProp ) );
}
    
    // @brief reordering operator:
    //        changes the order in which the directions are 
    //        stored in memory
    // 
    //        Perm
template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
    NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass> 
        NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass>::reorder
            (const pointND<NDIM,int> & Perm) 
{
  
}    
    
    // getSubObj returns a subObj region
    // of the given object extended from the
    // lower left corner ll to the upper right
    // corner ur
template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
    NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass> 
        NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass>::getSubObj
            (pointND<NDIM,float> ll, pointND<NDIM,float> ur)
{
  
}    
    
    // setSubMem sets a submemory region
    // of the given object with the values
    // given by the NDimMem object beginning
    // from the lower left corner ll of the 
    // given object
template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
    bool 
        NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass>::setSubObj
        (pointND<NDIM,float> ll, NDimObjBase & rhs)
{
  
}    
        
template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
    std::ostream& operator<<
        ( std::ostream & _os,NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass> & _Obj)
{
  _os<<(NDimDataBase<NDIM,DataType,MetaInfoClass> ) _Obj<<std::endl;
  _os<<_Obj.mObjProp;
  
  return _os;
}

#endif