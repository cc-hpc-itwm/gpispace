//
// C++ Interface: NDimObj
//
// Description: 
//
//
// Author: Daniel Gruenewald, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef NDIMOBJ_H
#define NDIMOBJ_H

/// standard includes
#include <stdlib.h>
#include <iostream>
#include <ostream>
#include <assert.h>

/// includes
#include "NDimObjBase.h"

/**
	@author Daniel Gruenewald

      class implements an N-dimensional Object
 */

template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
  class NDimObj : public NDimObjBase< NDIM
 				     ,DataType
                                     ,MetaInfoClass
				     ,ObjPropClass >
		 ,public Base_Obj   < NDimObj< NDIM
					      ,DataType
					      ,MetaInfoClass
					      ,ObjPropClass >
				     ,DataType >
{
       
  public:
        
    NDimObj(const char * pObjData, const ObjPropClass & ObjProp);
    
    // Make class zero copy capable
    // assignment operator
    template<typename T1, typename T2,operation_type OP>
      NDimObj(const Operator_Triple<T1,T2,OP>& X);
    // copy constructor    
    template<typename T1, typename T2,operation_type OP>
      const NDimObj & operator=(const Operator_Triple<T1,T2,OP> & X);
    
    // cast operator
    operator const NDimData<NDIM,DataType,MetaInfoClass>*() const;
    
    inline static ObjPropClass getObjProp(const NDimObj<NDIM,DataType,MetaInfoClass,ObjPropClass> &);
    
};
    

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//                                                        //
//             NDimObj implementation                     //
//                                                        //
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
    NDimObj<NDIM,DataType,MetaInfoClass,ObjPropClass>::NDimObj
        ( const char * _pObjData, const ObjPropClass & _ObjProp)
  : NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass>(_pObjData,_ObjProp)
{
  
}
 
template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
  template <typename T1, typename T2,operation_type OP> 
 NDimObj<NDIM,DataType,MetaInfoClass,ObjPropClass>::NDimObj(const Operator_Triple<T1,T2,OP> & X)
 {
   #ifdef DEBUG
   std::cout<<"NDimObj<NDIM,DataType,MetaInfoClass,ObjPropClass>::NDimObj(const Operator_Triple<T1,T2,OP> & X)"<<std::endl;
   #endif
   operator=(X);
 }
 
template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass> 
 template <typename T1, typename T2, operation_type OP>
  const NDimObj<NDIM,DataType,MetaInfoClass,ObjPropClass>& 
    NDimObj<NDIM,DataType,MetaInfoClass,ObjPropClass>::operator=(const Operator_Triple<T1,T2,OP> & X)
 {
   #ifdef DEBUG
   std::cout<<"NDimData<"<<NDIM<<","<<typeid(DataType).name()
   <<","<<typeid(MetaInfoClass).name()<<">::operator=(const Operator_Triple<T1,T2,OP> & X)"<<std::endl;
   #endif
   
   NDimDataBase<NDIM,DataType,MetaInfoClass> * pNDimDataBase = 
      static_cast     <NDimDataBase<NDIM,DataType,MetaInfoClass> * >(this);
   NDimData    <NDIM,DataType,MetaInfoClass> * pNDimData    = 
      reinterpret_cast<NDimData    <NDIM,DataType,MetaInfoClass> * >(pNDimDataBase);
   
   *pNDimData = (generate_new_operator_triple< NDimData<NDIM,DataType,MetaInfoClass>
					      ,Operator_Triple<T1,T2,OP> >::resolve(X) );
   
   NDimObjBase<NDIM,DataType,MetaInfoClass,ObjPropClass>::mObjProp = 
      X.template resolve< ObjPropClass
			 ,NDimObj<NDIM,DataType,MetaInfoClass,ObjPropClass> 
			 ,& NDimObj<NDIM,DataType,MetaInfoClass,ObjPropClass>::getObjProp >();
   
   return *this;
 }
 
 
 // cast operator
template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
  NDimObj<NDIM,DataType,MetaInfoClass,ObjPropClass>::
    operator const NDimData<NDIM,DataType,MetaInfoClass>*() const
 {
   const NDimDataBase<NDIM,DataType,MetaInfoClass> * pNDimDataBase = 
      static_cast     <const NDimDataBase<NDIM,DataType,MetaInfoClass> * >(this);
   const NDimData    <NDIM,DataType,MetaInfoClass> * pNDimData     = 
      reinterpret_cast<const NDimData    <NDIM,DataType,MetaInfoClass> * >(pNDimDataBase);
   return pNDimData;
 }
 
template <short NDIM, typename DataType, typename MetaInfoClass, typename ObjPropClass>
 inline ObjPropClass 
  NDimObj<NDIM,DataType,MetaInfoClass,ObjPropClass>::getObjProp(const NDimObj<NDIM,DataType,MetaInfoClass,ObjPropClass> & Obj)
 {
   return Obj.mObjProp;
 }

#endif