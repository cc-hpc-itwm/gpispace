//
// C++ Interface: NDimData
//
// Description: Adds zero copy / single loop functionality to the 
//		NDimDataBase class
//
//
// Author: Daniel Gruenewald, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef NDIMDATA_H
#define NDIMDATA_H

/// standard includes
#include <stdlib.h>
#include <iostream>
#include <ostream>
#include <assert.h>
#include <typeinfo>

/// includes
#include "NDimMemBase.h"

/**
	@author Daniel Gruenewald

      class supplements NDimDataBase with zero copy / single
      loop functionality
 */

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//                                                        //
//               NDimData Declaration                     //
//                                                        //
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

template <short NDIM, typename DataType, typename MetaInfoClass>
  class NDimData : public NDimMemBase < NDIM
				       ,DataType
				       ,NDimData< NDIM
						 ,DataType
						 ,MetaInfoClass> >
		  ,public Base_Obj    < NDimData< NDIM
					         ,DataType
					         ,MetaInfoClass>
                                       ,DataType >
			    
{
      typedef NDimMemBase<NDIM,DataType,NDimData<NDIM,DataType,MetaInfoClass> > base;
    
      public:
	
	// standard constructor
	NDimData();
	
	// default constructor
        NDimData( char * const _pData
		 ,const typename base::DatPntType  _pFirstElem
		 ,const pointND<NDIM,int> & _N);

        NDimData( char * const _pData
        		 ,const unsigned long offset
        		 ,const typename base::DatPntType _pFirstElem
        		 ,const pointND<NDIM,int> & _N);
		 
	// destructor	 
        ~NDimData();
		 
	 // dereference operator
	 NDimData * operator->();
	
	 // assignment operator for const NDimData rhs -> data is copied
	 NDimData & operator=(const NDimData &);
	 // copy constructor for const rhs -> const assignment is called
	 NDimData(const NDimData &);
	 // assignment operator for NDimData rhs -> data is not copied, ptrs
	 // point on the data of the rhs
	 NDimData & operator=(NDimData &);
	 // copy constructor for NDimData rhs -> assignemnt is called
	 NDimData(NDimData &);
	 
        // Make class zero copy capable
        // assignment operator
        template<typename T1, typename T2,operation_type OP>
            NDimData(const Operator_Triple<T1,T2,OP>& X);
        // copy constructor    
        template<typename T1, typename T2,operation_type OP>
            const NDimData& operator=(const Operator_Triple<T1,T2,OP> & X);
    
	inline NDimData * duplicateObj() const;    
	    
	
	// special functions for NDimData
	
	// equality operator
	bool operator==(const NDimData &) const;
	// unequality operator
	bool operator!=(const NDimData &) const;
	    
	// outstream operator <<
	template <short NDIM_OS, typename DataType_OS, typename MetaInfoClass_OS>
	   friend std::ostream& operator<<( std::ostream &
	        ,const NDimData<NDIM_OS,DataType_OS,MetaInfoClass_OS> &);
        
	MetaInfoClass getMetaInfo();
	
	void SetDataMemAddr(char * _CharPtr);
	
      private:
        
	// cast operator
	operator NDimMem<NDIM,DataType>();
        // const cast operator
	operator const NDimMem<NDIM,DataType>() const;
	
      private:
    
        // MetaInformation
        MetaInfoClass mMetaInfo;

};

template <short NDIM,typename DataType, class MetaInfoClass>
struct DataTypeDescr<NDimData<NDIM,DataType,MetaInfoClass> > {
  typedef NDimData<NDIM,DataType,MetaInfoClass> value_type;
  typedef NDimObjPtr<NDIM, NDimData<NDIM,DataType,MetaInfoClass> > pointer_type;
  typedef NDimData<NDIM,DataType,MetaInfoClass> & reference_type;
  typedef NDimData<NDIM,DataType,MetaInfoClass>   reference_return_type;
  static const bool is_standard_type = false;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//                                                        //
//            NDimData implementation                     //
//                                                        //
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

template <short NDIM, typename DataType, typename MetaInfoClass>
    NDimData<NDIM,DataType,MetaInfoClass>::NDimData()
  : base()
   ,mMetaInfo()
{
  #ifdef DEBUG
    std::cout<<"NDimData<"<<NDIM<<","<<typeid(DataType).name()
	<<","<<typeid(MetaInfoClass).name()<<">::NDimData()"<<std::endl;
  #endif
}

template <short NDIM, typename DataType, typename MetaInfoClass>
    NDimData<NDIM,DataType,MetaInfoClass>::NDimData( char * const _pData
                                                    ,const typename base::DatPntType _pFirstElem
                                                    ,const pointND<NDIM,int> & _N)
    :  mMetaInfo(_pData)
      ,base( _pData
	    ,MetaInfoClass::getSize()
	    ,_pFirstElem
	    ,_N )
{
  #ifdef DEBUG
    std::cout<<"NDimData<"<<NDIM<<","<<typeid(DataType).name()
	<<","<<typeid(MetaInfoClass).name()<<">::NDimData(const char * _pData,const pointND<NDIM,int> & _N)"<<std::endl;
  #endif
}

template <short NDIM, typename DataType, typename MetaInfoClass>
    NDimData<NDIM,DataType,MetaInfoClass>::NDimData( char * const _pData
					    ,const unsigned long _offset
					    ,const typename base::DatPntType _pT
					    ,const pointND<NDIM,int> &_N)
  : mMetaInfo(_pData)
   ,base( _pData
		    ,MetaInfoClass::getSize()
		    ,_pT
		    ,_N )
		    {

		    }


template <short NDIM, typename DataType, typename MetaInfoClass>
  NDimData<NDIM,DataType,MetaInfoClass>::~NDimData()
{
}

// dereference operator
template <short NDIM, typename DataType, class MetaInfoClass>
  NDimData<NDIM,DataType,MetaInfoClass> *
    NDimData<NDIM,DataType,MetaInfoClass>::operator->()
{
  return this;
}

// assignment operator for const rhs
template <short NDIM, typename DataType, class MetaInfoClass>
  NDimData<NDIM,DataType,MetaInfoClass> &
    NDimData<NDIM,DataType,MetaInfoClass>::operator=(const NDimData & rhs)
{
  
  if( this != rhs.getConstRawPtr() )
  { 
    base::operator=(rhs);
    mMetaInfo.setMetaInfo(base::getpData());
    mMetaInfo=rhs.mMetaInfo;
  }
  
  return *this;
  
}

// copy constructor for const rhs
template <short NDIM, typename DataType, typename MetaInfoClass>
  NDimData<NDIM,DataType,MetaInfoClass>::NDimData(const NDimData & rhs)
  : base(rhs)
{
  mMetaInfo.setMetaInfo(base::getpData());
  mMetaInfo=rhs.mMetaInfo;
}

// assignment operator for rhs
template <short NDIM, typename DataType, typename MetaInfoClass>
  NDimData<NDIM,DataType,MetaInfoClass> &
    NDimData<NDIM,DataType,MetaInfoClass>::operator=(NDimData & rhs)
{
  
  if( this != rhs.getRawPtr() )
  { 
    base::operator=(rhs);
    mMetaInfo.setMetaInfo(base::getpData());
    mMetaInfo=rhs.mMetaInfo;
  }
  
  return *this;
  
}

// copy constructor for rhs
template <short NDIM, typename DataType, typename MetaInfoClass>
  NDimData<NDIM,DataType,MetaInfoClass>::NDimData(NDimData & rhs)
  : base(rhs)
{
  mMetaInfo.setMetaInfo(base::getpData());
  mMetaInfo=rhs.mMetaInfo;
}

// assignment operator
// template <short NDIM, typename DataType, typename MetaInfoClass>
//   NDimData<NDIM,DataType,MetaInfoClass> & 
//     NDimData<NDIM,DataType,MetaInfoClass>::operator=( const NDimData<NDIM,DataType,MetaInfoClass> & _rhs)
// {
//   #ifdef DEBUG
//   std::cout<<"NDimDataBase<"<<NDIM<<","<<typeid(DataType).name()
//   <<","<<typeid(MetaInfoClass).name()<<">::operator="<<std::endl;
//   #endif
//   if( this->operator&() != _rhs.operator&() )
//   { 
//     base::operator=(_rhs);
//     mMetaInfo.setMetaInfo(base::getpData());
//     mMetaInfo=_rhs.mMetaInfo;
//   }
//   
//   return *this;
// }
// 
// // copy constructor
// template <short NDIM, typename DataType, typename MetaInfoClass>
//   NDimData<NDIM,DataType,MetaInfoClass>::NDimData(const NDimData<NDIM,DataType,MetaInfoClass> & _rhs)
// {
//   #ifdef DEBUG
//   std::cout<<std::endl;
//   std::cout<<"NDimData<"<<NDIM<<","<<typeid(DataType).name()
//   <<","<<typeid(MetaInfoClass).name()<<">::copy constr."<<std::endl;
//   #endif
//   
//   operator=(_rhs);
//   
//   #ifdef DEBUG
//   std::cout<<"NDimData<"<<NDIM<<","<<typeid(DataType).name()
//   <<","<<typeid(MetaInfoClass).name()<<">::copy constr. finished"<<std::endl;
//   std::cout<<std::endl;
//   #endif
// }

// equality operator
template <short NDIM, typename DataType, typename MetaInfoClass>
  bool 
    NDimData<NDIM,DataType,MetaInfoClass>::operator==(const NDimData & _rhs) const
{
  return ( ( base::operator==(_rhs) ) && 
           ( mMetaInfo == _rhs.mMetaInfo ) );
}

// un equality operator
template <short NDIM, typename DataType, typename MetaInfoClass>
  bool
    NDimData<NDIM,DataType,MetaInfoClass>::operator!=(const NDimData & _rhs) const
{
  return ( ! (*this == _rhs) );
}

// template <short NDIM, typename DataType, typename MetaInfoClass>
//     NDimData<NDIM,DataType,MetaInfoClass>::NDimData
//         (const NDimMem<NDIM,DataType> & _NDimMem, const MetaInfoClass & _MetaInfo)
//   : NDimDataBase<NDIM,DataType,MetaInfoClass>(_NDimMem,_MetaInfo)
// {
// #ifdef DEBUG
//   std::cout<<"NDimData<"<<NDIM<<","<<typeid(DataType).name()
//       <<","<<typeid(MetaInfoClass).name()<<">::NDimData(const NDimMem<NDIM,DataType> & _NDimMem, const MetaInfoClass & _MetaInfo)"<<std::endl;
// #endif
// }

template <short NDIM, typename DataType, typename MetaInfoClass>
  template <typename T1, typename T2,operation_type OP> 
    NDimData<NDIM,DataType,MetaInfoClass>::NDimData(const Operator_Triple<T1,T2,OP> & X)
{
  #ifdef DEBUG
    std::cout<<"NDimData<NDIM,DataType,MetaInfoClass>::NDimData(const Operator_Triple<T1,T2,OP> & X)"<<std::endl;
  #endif
  operator=(X);
}

template <short NDIM, typename DataType, typename MetaInfoClass> 
    template <typename T1, typename T2, operation_type OP>
        const NDimData<NDIM,DataType,MetaInfoClass>& 
            NDimData<NDIM,DataType,MetaInfoClass>::operator=(const Operator_Triple<T1,T2,OP> & X)
{
  #ifdef DEBUG
    std::cout<<"NDimData<"<<NDIM<<","<<typeid(DataType).name()
	<<","<<typeid(MetaInfoClass).name()<<">::operator=(const Operator_Triple<T1,T2,OP> & X)"<<std::endl;
  #endif
   			     
    const NDimData<NDIM,DataType,MetaInfoClass> * VecPtr[X.getNVecElem()];
    
    X.getVecElemPtrs(VecPtr);
    
#ifndef NDEBUG
    // ... check that all NDimMemBase objects are equal ...
    for(int i = 1; i < X.getNVecElem(); ++i)
    {
       //if( !( *(VecPtr[i]) == *(VecPtr[i-1]) ) )
       //{
       //	     std::runtime_error("Operands do not match");
       //}
    }	
#endif

    bool self_assignment = false;
    bool proceed = true;

    for( int i = 0; i < X.getNVecElem(); ++i)
    {
    	if ( this == VecPtr[i] ) self_assignment = true;
    }

    if( ! self_assignment )
    {
    	*this = *VecPtr[0];
    }

    NDimMem<NDIM,DataType> * pNDimMemRes;

    pNDimMemRes =
    		new NDimMem<NDIM,DataType>( this->getpData()
    				,this->getOffset()
    				,this->getpT()
    				,this->getN() );

    const MetaInfoClass * MetaInfoPtr[X.getNVecElem()];
    const NDimMem<NDIM,DataType> * NDimMemPtr[X.getNVecElem()];
    
    for( int i (0) ; i < X.getNVecElem() ; ++i )
    {
      MetaInfoPtr[i] = &(VecPtr[i]->mMetaInfo);
      NDimMemPtr [i] = new NDimMem<NDIM,DataType>( VecPtr[i]->getpData()
    		                                        ,VecPtr[i]->getOffset()
    		                                        ,VecPtr[i]->getpT()
    		                                        ,VecPtr[i]->getN());
    }

    MetaInfoClass MetaInfo_tmp    = OpTripleFuncs<Operator_Triple<T1,T2,OP> >
                        ::template resolve<const MetaInfoClass>(MetaInfoPtr,X);
    
    *pNDimMemRes = OpTripleFuncs<Operator_Triple<T1,T2,OP> >
                                ::resolve(NDimMemPtr,X);

//    for(int i (0) ; i < X.getNVecElem(); ++i)
//    {
//    	  std::cout<<"NDimData::operator=(Operator_Triple): NDimMemPtr["<<i<<"] ="
//    			   <<(void *)NDimMemPtr[i]<<std::endl;
//    }
//    NDimMem<NDIM,DataType> Res = OpTripleFuncs<Operator_Triple<T1,T2,OP> >
//                                    ::resolve(NDimMemPtr,X);
//    std::cout<<"Res = "<<std::endl;
//    std::cout<<Res<<std::endl<<std::endl;

//    SetDataMemAddr(pNDimMemRes->getpData());
    mMetaInfo = MetaInfo_tmp;

    // clean up the helper objects
    for( int i (0) ; i < X.getNVecElem() ; ++i )
    {
      delete NDimMemPtr[i];
    }
    delete pNDimMemRes;
    
  return *this;
}

// outstream operator
template <short NDIM, typename DataType,typename MetaInfoClass>
std::ostream& operator<<(std::ostream & os, const NDimData<NDIM,DataType,MetaInfoClass> & _M)
{ 
  os<<_M.mMetaInfo;
  os<<(typename NDimData<NDIM,DataType,MetaInfoClass>::base)_M<<std::endl;
  
  return os;
}

// cast operator
template <short NDIM, typename DataType, typename MetaInfoClass>
  NDimData<NDIM,DataType,MetaInfoClass>::
    operator NDimMem<NDIM,DataType>() 
{
  
  NDimMem<NDIM,DataType> Res(this->getpT(),this->getN());
  
  return Res;
}

// const cast operator
template <short NDIM, typename DataType, typename MetaInfoClass>
  NDimData<NDIM,DataType,MetaInfoClass>::
   operator const NDimMem<NDIM,DataType>() const
{  
  
   const NDimMem<NDIM,DataType> Res(this->getpT(),this->getN());
  
   return Res;
}

template <short NDIM, typename DataType, typename MetaInfoClass>
  inline
    NDimData<NDIM,DataType,MetaInfoClass> *
      NDimData<NDIM,DataType,MetaInfoClass>::duplicateObj() const
{
  
  NDimData<NDIM,DataType,MetaInfoClass> * pObj 
    = new NDimData<NDIM,DataType,MetaInfoClass>
      ( base::getpData()
       ,base::pT
       ,base::Nlat);
  
  return pObj;
  
}

template <short NDIM, typename DataType, typename MetaInfoClass>
  inline MetaInfoClass 
    NDimData<NDIM,DataType,MetaInfoClass>::getMetaInfo()
{
  return mMetaInfo;
}

template <short NDIM, typename DataType, typename MetaInfoClass>
  inline void 
    NDimData<NDIM,DataType,MetaInfoClass>::SetDataMemAddr(char * _CharPtr)
{
  mMetaInfo.setMetaInfo(_CharPtr);
  base::SetDataMemAddr(_CharPtr);
}
#endif
