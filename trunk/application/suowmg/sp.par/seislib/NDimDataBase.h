//
// C++ Interface: NDimDataBaseBase
//
// Description: 
//
//
// Author: Daniel Gruenewald, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef NDIMDATABASE_H
#define NDIMDATABASE_H

/// standard includes
#include <stdlib.h>
#include <iostream>
#include <ostream>
#include <assert.h>
#include <typeinfo>

/// includes
#include "pointND.h"
#include "NDimMemBase.h"
#include "MetaInfo.h"
#include "NDimDataPtr.h"

/**
	@author Daniel Gruenewald

      class implements an N-dimensional data set.
      A data set is assumed to consist out of some 
      metainformation and the actual data. 
 */

template <short NDIM, typename DataType, typename MetaInfoClass>
     class NDimDataBase : public NDimMemBase<NDIM,DataType>
{
   /// Member functions
    
      public:
	
	typedef typename DataTypeDescr<DataType>::value_type   			DatValType;
	typedef typename DataTypeDescr<DataType>::pointer_type 			DatPntType;
	typedef typename DataTypeDescr<DataType>::reference_type    		DatRefType;
	typedef typename DataTypeDescr<DataType>::reference_return_type 	DatRefRetType;
	typedef MetaInfoClass	MetaInfoType;
       
        NDimDataBase( char * const _pMetaInfo
		     ,const DatPntType _pFirstElem
                     ,const pointND<NDIM,int> & _N);
        ~NDimDataBase();

	// reference operator
	NDimDataPtr<NDIM,NDimDataBase> operator&();
	// const reference operator
	const NDimDataPtr<NDIM,NDimDataBase> operator&() const;
	
	// dereference operator
	NDimDataBase * operator->();
	
        // assignment operator
        NDimDataBase & operator=(const NDimDataBase &);
        // copy constructor
        NDimDataBase( const NDimDataBase &);
            
        // equality operator
        bool operator==(const NDimDataBase &) const;
	
	// cast operator
	operator const NDimMem<NDIM,DataType>*() const;
        
        // operator <<
        template <short NDIM_OS, typename DataType_OS, typename MetaInfoClass_OS>
            friend std::ostream& operator<<( std::ostream &
                ,const NDimDataBase<NDIM_OS,DataType_OS,MetaInfoClass_OS> &);
      
	size_t getObjMemSize() const;
	
	void SetDataMemAddr(char * _CharPtr);
	
      protected:
        
        NDimDataBase();
        NDimDataBase(const NDimMem<NDIM,DataType> &, const MetaInfoClass &);
      
      private:
	
	
      /// Member data
      
       protected:  
         
        // pointer to the data
        //char * pData;
        
        // MetaInformation
        MetaInfoClass mMetaInfo;
        
        //bool selfalloc;
};

template <short NDIM,typename DataType, class MetaInfoClass>
struct DataTypeDescr<NDimDataBase<NDIM,DataType,MetaInfoClass> > {
  typedef NDimDataBase<NDIM,DataType,MetaInfoClass> value_type;
  typedef NDimDataPtr<NDIM, NDimDataBase<NDIM,DataType,MetaInfoClass> > pointer_type;
  typedef NDimDataBase<NDIM,DataType,MetaInfoClass> & reference_type;
  typedef NDimDataBase<NDIM,DataType,MetaInfoClass>   reference_return_type;
  static const bool is_standard_type = false;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//                                                        //
//          NDimDataBase implementation                   //
//                                                        //
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

template <short NDIM, typename DataType, typename MetaInfoClass>
    NDimDataBase<NDIM,DataType,MetaInfoClass>::NDimDataBase()
  : //pData(NULL)
   mMetaInfo()
   ,NDimMemBase<NDIM,DataType>()
   //,selfalloc(false)
{
  #ifdef DEBUG
    std::cout<<"NDimDataBase<"<<NDIM<<","<<typeid(DataType).name()
	<<","<<typeid(MetaInfoClass).name()<<">::NDimDataBase()"<<std::endl;
  #endif
}

template <short NDIM, typename DataType, typename MetaInfoClass>
    NDimDataBase<NDIM,DataType,MetaInfoClass>::NDimDataBase( char * const _pData
							    ,const DatPntType _pFirstElem
							    ,const pointND<NDIM,int> & _N)
  : //pData(_pData) 
    mMetaInfo(_pData)
   ,NDimMemBase<NDIM,DataType>( _pData
			       ,MetaInfoClass::getSize()
			       ,_pFirstElem
   			       ,_N )
   //,selfalloc(false)
{
#ifdef DEBUG
  std::cout<<"NDimDataBase<"<<NDIM<<","<<typeid(DataType).name()
      <<","<<typeid(MetaInfoClass).name()<<">::NDimDataBase(const char * _pData,const pointND<NDIM,int> & _N)"<<std::endl;
  std::cout<<"_pFirstElem[0] = "<<_pFirstElem[0]<<std::endl;
#endif
}

template <short NDIM, typename DataType, typename MetaInfoClass>
    NDimDataBase<NDIM,DataType,MetaInfoClass>::NDimDataBase
        (const NDimMem<NDIM,DataType> & _NDimMem, const MetaInfoClass & _MetaInfo)
  : //pData((char *)_MetaInfo.getMetaInfo())
    mMetaInfo(_MetaInfo)
   ,NDimMemBase<NDIM,DataType>(_NDimMem)
   //,selfalloc(false)
{
#ifdef DEBUG
  std::cout<<"NDimDataBase<"<<NDIM<<","<<typeid(DataType).name()
      <<","<<typeid(MetaInfoClass).name()<<">::NDimDataBase(const NDimMem<NDIM,DataType> & _NDimMem, const MetaInfoClass & _MetaInfo)"<<std::endl;
#endif
}

template <short NDIM, typename DataType, class MetaInfoClass>
    NDimDataBase<NDIM,DataType,MetaInfoClass>::~NDimDataBase()
{
//   if(selfalloc)
//   {
//     if(pData != NULL)
//       delete[] pData;
//   }
}

// reference operator
template <short NDIM, typename DataType, class MetaInfoClass>
  NDimDataPtr<NDIM,NDimDataBase<NDIM,DataType,MetaInfoClass> > 
    NDimDataBase<NDIM,DataType,MetaInfoClass>::operator&()
{
#ifdef DEBUG
  std::cout<<"NDimDataBase<NDIM,DataType>::operator&()"<<std::endl;
#endif
  return NDimDataPtr<NDIM,NDimDataBase<NDIM,DataType,MetaInfoClass> >
    (NDimMemBase<NDIM,DataType>::pData 
    ,NDimMemBase<NDIM,DataType>::pT
    ,NDimMemBase<NDIM,DataType>::Nlat
    ,NDimMemBase<NDIM,DataType>::Ntot);
}

// const reference operator
template <short NDIM, typename DataType, class MetaInfoClass>
  const NDimDataPtr<NDIM,NDimDataBase<NDIM,DataType,MetaInfoClass> > 
    NDimDataBase<NDIM,DataType,MetaInfoClass>::operator&() const
{
#ifdef DEBUG
  std::cout<<"NDimDataBase<NDIM,DataType>::operator&()"<<std::endl;
#endif
  return NDimDataPtr<NDIM,NDimDataBase<NDIM,DataType,MetaInfoClass> >
    (NDimMemBase<NDIM,DataType>::pData 
    ,NDimMemBase<NDIM,DataType>::pT
    ,NDimMemBase<NDIM,DataType>::Nlat
    ,NDimMemBase<NDIM,DataType>::Ntot);
}

// dereference operator
template <short NDIM, typename DataType,class MetaInfoClass>
  NDimDataBase<NDIM,DataType,MetaInfoClass> *
    NDimDataBase<NDIM,DataType,MetaInfoClass>::operator->()
{
  return this;
}

// // pointwise addition
// template <short NDIM, typename DataType, typename MetaInfoClass>
//     NDimDataBase<NDIM,DataType,MetaInfoClass> & 
//         NDimDataBase<NDIM,DataType,MetaInfoClass>::operator+
//             (const NDimDataBase<NDIM,DataType,MetaInfoClass> & rhs) const
// {
//   
//   NDimDataBase<NDIM,DataType,MetaInfoClass>   res( mNDimMem + rhs.mNDimMem
//                                               ,mMetaInfo + rhs.mMetaInfo);
//   
//   return res;
// }
//         
// // pointwise subtraction
// template <short NDIM, typename DataType, typename MetaInfoClass>
//     NDimDataBase<NDIM,DataType,MetaInfoClass> &
//         NDimDataBase<NDIM,DataType,MetaInfoClass>::operator-
//             (const NDimDataBase<NDIM,DataType,MetaInfoClass> & rhs) const
// {
//   
//   NDimDataBase<NDIM,DataType,MetaInfoClass>   res( mNDimMem - rhs.mNDimMem
//                                               ,mMetaInfo - rhs.mMetaInfo);
//   
//   return res;
// }
//         
// // pointwise multiplication
// template <short NDIM, typename DataType, typename MetaInfoClass>
//     NDimDataBase<NDIM,DataType,MetaInfoClass> &
//         NDimDataBase<NDIM,DataType,MetaInfoClass>::operator*
//             (const NDimDataBase<NDIM,DataType,MetaInfoClass> & rhs) const
// {
//   NDimDataBase<NDIM,DataType,MetaInfoClass>   res( mNDimMem * rhs.mNDimMem
//                                               ,mMetaInfo * rhs.mMetaInfo);
//       
//   return res;
// }
//    
// // scalar multiplication
// template <short NDIM, typename DataType, typename MetaInfoClass>
//     NDimDataBase<NDIM,DataType,MetaInfoClass> &
//         NDimDataBase<NDIM,DataType,MetaInfoClass>::operator*
//             (const DataType & rhs) const
// {
//   NDimDataBase<NDIM,DataType,MetaInfoClass>   res( mNDimMem * rhs
//                                               ,mMetaInfo * rhs);
//   
//   return res;
// }

// assignment operator
template <short NDIM, typename DataType, typename MetaInfoClass>
    NDimDataBase<NDIM,DataType,MetaInfoClass> & 
        NDimDataBase<NDIM,DataType,MetaInfoClass>::operator=
            ( const NDimDataBase<NDIM,DataType,MetaInfoClass> & rhs)
{
  #ifdef DEBUG
    std::cout<<"NDimDataBase<"<<NDIM<<","<<typeid(DataType).name()
	<<","<<typeid(MetaInfoClass).name()<<">::operator="<<std::endl;
  #endif
  if( this->operator&() != rhs.operator&() )
  { 
    NDimMemBase<NDIM,DataType>::operator=(rhs);
    mMetaInfo.setMetaInfo(NDimMemBase<NDIM,DataType>::getpData());
    mMetaInfo=rhs.mMetaInfo;
  }
  
  return *this;
}

// copy constructor
template <short NDIM, typename DataType, typename MetaInfoClass>
    NDimDataBase<NDIM,DataType,MetaInfoClass>::NDimDataBase
        ( const NDimDataBase<NDIM,DataType,MetaInfoClass> & rhs)
{
  #ifdef DEBUG
    std::cout<<std::endl;
    std::cout<<"NDimDataBase<"<<NDIM<<","<<typeid(DataType).name()
	<<","<<typeid(MetaInfoClass).name()<<">::copy constr."<<std::endl;
  #endif
   
   operator=(rhs);
  
  #ifdef DEBUG
    std::cout<<"NDimDataBase<"<<NDIM<<","<<typeid(DataType).name()
	<<","<<typeid(MetaInfoClass).name()<<">::copy constr. finished"<<std::endl;
    std::cout<<std::endl;
  #endif
}

// equality operator
template <short NDIM, typename DataType, typename MetaInfoClass>
    bool 
        NDimDataBase<NDIM,DataType,MetaInfoClass>::operator==
        (const NDimDataBase & rhs) const
{
  return ( ( NDimMemBase<NDIM,DataType>::operator==(rhs) ) && 
	   ( mMetaInfo == rhs.mMetaInfo ) );
}

// outstream operator
template <short NDIM, typename DataType,typename MetaInfoClass>
    std::ostream& operator<<(std::ostream & os, const NDimDataBase<NDIM,DataType,MetaInfoClass> & _M)
{ 
  os<<_M.mMetaInfo;
  os<<(const NDimMemBase<NDIM,DataType>)_M<<std::endl;
  
  return os;
}

template <short NDIM, typename DataType, typename MetaInfoClass>
  size_t
    NDimDataBase<NDIM,DataType,MetaInfoClass>::getObjMemSize() const
{
  return NDimMemBase<NDIM,DataType>::getObjMemSize();
}

template <short NDIM, typename DataType, typename MetaInfoClass>
  inline void 
    NDimDataBase<NDIM,DataType,MetaInfoClass>::SetDataMemAddr(char * _CharPtr)
{
  //pData = _CharPtr;
  mMetaInfo.setMetaInfo(_CharPtr);
  NDimMemBase<NDIM,DataType>::SetDataMemAddr(_CharPtr);
}

#endif