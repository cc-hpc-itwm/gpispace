//
// C++ Interface: NDimDataPtr
//
// Description: 
//
//
// Author: Daniel Gruenewald, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef NDIMDATAPTR
#define NDIMDATAPTR

/// standard includes
#include <stdlib.h>
#include <iostream>
#include <ostream>
#include <assert.h>

/// user defined includes
#include "pointND.h"

#include <iterator>
using namespace std;


/**
@author Daniel Gruenewald

\brief class implements a pointer class to NDimData(Base)
\details
\pre
*/

template <short NDIM, class NDimObj>
class NDimDataPtr {
  
  typedef typename NDimObj::DatValType ValType;
  typedef typename NDimObj::DatPntType PntType;
  typedef typename NDimObj::DatRefType RefType;
  typedef typename NDimObj::MetaInfoType MetaInfoType;
  
  /// Methods
  
  public: 
    
    NDimDataPtr();						// Default constructor
    NDimDataPtr( char * _pMetaInfo
		,PntType _pData
		,const pointND<NDIM,int> & _Next
		,const int & _Ntot);				// constructor
    ~NDimDataPtr();						// destructor
    
    NDimDataPtr & operator=(const NDimDataPtr & rhs);		// assignment operator
    NDimDataPtr(const NDimDataPtr & rhs);			// copy constructor
    
    NDimObj operator*() const;				// derefence operator
    NDimObj operator->() const;				// operator ->
    
    NDimObj operator[](const int & idx) const;	        // access operator
    NDimDataPtr operator+(const int & n) const;		// add an integer of n elements to the pointer
    NDimDataPtr operator-(const int & n) const;		// subtract an integer of n elements from the pointer
    
    NDimDataPtr & operator++();				// prefix operator
    NDimDataPtr   operator++(int);				// postfix operator
    
    bool operator==(const NDimDataPtr & rhs) const;		// equality operator
    bool operator!=(const NDimDataPtr & rhs) const;		// not equal to operator
    
    operator void*() const;				// cast operator to void * 
							// Required in order to compare to NULL
    operator char*() const;				// cast operator to char *
    
    void SetDataMemAddr(char * _CharPtr);
    
    NDimObj * getObjPtr();
    
  private:
    
    void SetDataMemAddr(char * _CharPtr, Int2Type<true>);
    void SetDataMemAddr(char * _CharPtr, Int2Type<false>);
    
  /// Member variables:
    
  private:
    
    char * pMetaInfo;					// pointer to the actual MetaInfoData
    PntType pData;     					// pointer to the actual data
    pointND<NDIM,int> Next;				// lattice extension of the N-dimensional object
    int Ntot;						// total number of lattice points
    
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//                                                         //
//               NDimDataPtr Implementation                //
//                                                         //
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

// default constructor
template <short NDIM, typename NDimObj>
  NDimDataPtr<NDIM,NDimObj>::NDimDataPtr()
  : pMetaInfo(NULL),pData(NULL),Next(),Ntot(0)
{
  
}

template <short NDIM, typename NDimObj>
  NDimDataPtr<NDIM,NDimObj>::NDimDataPtr( char * _pMetaInfo
					 ,PntType _pData
					 ,const pointND<NDIM,int> & _Next
					 ,const int & _Ntot)
  : pMetaInfo(_pMetaInfo),pData(_pData),Next(_Next),Ntot(_Ntot)
{
#ifdef DEBUG  
  std::cout<<"NDimDataPtr<NDIM,NDimObj> construction with:"<<std::endl;
  std::cout<<"_pMetaInfo  = "<<(void *)_pMetaInfo<<std::endl;
  std::cout<<"_pData      = "<<(void *)_pData<<std::endl;
  std::cout<<"_Next       = "<<_Next<<std::endl;
  std::cout<<"_Ntot       = "<<_Ntot<<std::endl;
#endif   
}

// destructor
template <short NDIM, typename NDimObj>
  NDimDataPtr<NDIM,NDimObj>::~NDimDataPtr()
{
  
}

// assignment operator
template <short NDIM, typename NDimObj>
  NDimDataPtr<NDIM,NDimObj> & 
    NDimDataPtr<NDIM,NDimObj>::operator=(const NDimDataPtr & rhs)
{
  if ( this != &rhs)
  {
    pMetaInfo = rhs.pMetaInfo;
    pData     = rhs.pData;
    Ntot      = rhs.Ntot;
    Next      = rhs.Next;
  }
    
  return *this;
}

// copy constructor
template <short NDIM, typename NDimObj>
  NDimDataPtr<NDIM,NDimObj>::NDimDataPtr(const NDimDataPtr & rhs)
{
  operator=(rhs);
}

// dereference operator
template <short NDIM, typename NDimObj>
  NDimObj  
    NDimDataPtr<NDIM,NDimObj>::operator*() const
{
  #ifdef DEBUG
  std::cout<<"NDimDataPtr<NDIM,NDimObj> dereference:"<<std::endl;
  std::cout<<"pMetaInfo  = "<<(void *)pMetaInfo<<std::endl;
  std::cout<<"pData      = "<<(void *)pData<<std::endl;
  std::cout<<"Next       = "<<Next<<std::endl;
  #endif
  return NDimObj(pMetaInfo,pData,Next);
}

// operator ->
template <short NDIM, typename NDimObj>
  NDimObj 
    NDimDataPtr<NDIM,NDimObj>::operator->() const
{
  return operator*();
}

// access operator
template <short NDIM, typename NDimObj>
  NDimObj 
    NDimDataPtr<NDIM,NDimObj>::operator[](const int & idx) const
{
  const NDimDataPtr<NDIM,NDimObj> NDimDataPtrTmp(operator+(idx));
  
  return ( *NDimDataPtrTmp );
}

// add an integer of n elements to the pointer
template <short NDIM, typename NDimObj>
  NDimDataPtr<NDIM,NDimObj> 
    NDimDataPtr<NDIM,NDimObj>::operator+(const int & n) const
{
  size_t TotSize = (*this)->getObjMemSize();
 
  NDimDataPtr<NDIM,NDimObj> NDimDataPtrTmp(*this);
  
  NDimDataPtrTmp.SetDataMemAddr(pMetaInfo + n * TotSize);
  
  return NDimDataPtrTmp;
}

// subtract an integer of n elements from the pointer
template <short NDIM, typename NDimObj>
  NDimDataPtr<NDIM,NDimObj> 
    NDimDataPtr<NDIM,NDimObj>::operator-(const int & n) const
{
  size_t TotSize = (*this)->getObjMemSize();
  
  NDimDataPtr<NDIM,NDimObj> NDimDataPtrTmp(*this);
  
  NDimDataPtrTmp.SetDataMemAddr(pMetaInfo - n * TotSize);
  
  return NDimDataPtrTmp;
}

// prefix operator: ++NDimDataPtr
template <short NDIM, typename NDimObj>
  NDimDataPtr<NDIM,NDimObj> & 
    NDimDataPtr<NDIM,NDimObj>::operator++()				
{
  size_t TotSize = (*this)->getObjMemSize();
  
  SetDataMemAddr(pMetaInfo + TotSize);
  
  return *this;
  
}

// postfix operator: NDimDataPtr++
template <short NDIM, typename NDimObj>
  NDimDataPtr<NDIM,NDimObj>   
    NDimDataPtr<NDIM,NDimObj>::operator++(int)
{
  return operator++();
}

// equality operator
template <short NDIM, typename NDimObj>
  bool 
    NDimDataPtr<NDIM,NDimObj>::operator==(const NDimDataPtr & rhs) const
{
  const bool is_equal( ( pMetaInfo == rhs.pMetaInfo ) &&
		       ( pData == rhs.pData         ) &&
		       ( Next  == rhs.Next          ) &&
		       ( Ntot  == rhs.Ntot          )
		      );
  
  return is_equal;
  
}

// "not equal to" operator
template <short NDIM, typename NDimObj>
  bool 
    NDimDataPtr<NDIM,NDimObj>::operator!=(const NDimDataPtr & rhs) const
{
  return ( !( *this == rhs) );
}

template <short NDIM, typename NDimObj>
  NDimDataPtr<NDIM,NDimObj>::operator void*() const
{
  return ( (void *)pMetaInfo );
}

template <short NDIM, typename NDimObj>
  NDimDataPtr<NDIM,NDimObj>::operator char*() const
{
  return ( (char *)pMetaInfo );
}

template <short NDIM, typename NDimObj>
  inline void 
    NDimDataPtr<NDIM,NDimObj>::SetDataMemAddr(char * _CharPtr)
{
  SetDataMemAddr(_CharPtr, Int2Type<DataTypeDescr<ValType>::is_standard_type>() );
}

// the represented data type is a fundamental type
template <short NDIM, typename NDimObj>
  inline void 
    NDimDataPtr<NDIM,NDimObj>::SetDataMemAddr(char * _CharPtr, Int2Type<true>)
{
  pMetaInfo = _CharPtr;
  pData = (PntType)( _CharPtr + MetaInfoType::getSize() );
}

// the represented data type is not a fundamental type
template <short NDIM, typename NDimObj>
  inline void 
    NDimDataPtr<NDIM,NDimObj>::SetDataMemAddr(char * _CharPtr, Int2Type<false>)
{
  pMetaInfo = _CharPtr;
  pData.SetDataMemAddr( _CharPtr + MetaInfoType::getSize() );
}

template <short NDIM, typename NDimObj>
  inline NDimObj * 
    NDimDataPtr<NDIM,NDimObj>::getObjPtr()
{
  return new NDimObj(pMetaInfo,pData,Next);;
}
#endif