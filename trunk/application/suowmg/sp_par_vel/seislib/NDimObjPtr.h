//
// C++ Interface: NDimObjPtr
//
// Description: 
//
//
// Author: Daniel Gruenewald, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef NDIMOBJPTR_DEFINITION
#define NDIMOBJPTR_DEFINITION

/// standard includes
#include <stdlib.h>
#include <iostream>
#include <ostream>
#include <assert.h>

/// user defined includes
#include "pointND.h"

template <int v>
struct Int2Type
{
  enum {value = v};
};

template <typename T>
  struct DataTypeDescr 
{
  typedef T value_type;
  typedef T* pointer_type;
  typedef T& reference_type;
  typedef T& reference_return_type;
  static const bool is_standard_type = true;
};

template <typename T>
struct det
{
  inline static size_t MemSize(T & rhs)
  {
    return sizeof(T);
  }
};

/**
@author Daniel Gruenewald

\brief class implements a pointer class to NDimObj
\details
\pre
*/

template <short NDIM, class NDimObj>
class NDimObjPtr {
  
  typedef typename NDimObj::DatValType ValType;
  typedef typename NDimObj::DatPntType PntType;
  typedef typename NDimObj::DatRefType RefType;
  
  /// Methods
  
  public: 
    
    NDimObjPtr();					// Default constructor
    NDimObjPtr( char * _pData 
               ,const unsigned long & _offset
	       ,const PntType & _pT
               ,const pointND<NDIM,int> & _Next
	       ,const int & _Ntot);			// constructor
    ~NDimObjPtr();					// destructor
    
    NDimObjPtr & operator=(const NDimObjPtr & rhs);	// assignment operator
    NDimObjPtr(const NDimObjPtr & rhs);			// copy constructor
    
    NDimObj  operator*() const;			        // derefence operator
    //NDimObj operator*();
    NDimObj operator->() const;			        // operator ->
    //NDimObj operator->();
    
    NDimObj operator[](const int & idx) const;	        // access operator
    NDimObjPtr operator+(const int & n) const;		// add an integer of n elements to the pointer
    NDimObjPtr operator-(const int & n) const;		// subtract an integer of n elements from the pointer
    
    NDimObjPtr & operator++();				// prefix operator
    NDimObjPtr   operator++(int);			// postfix operator
    
    bool operator==(const NDimObjPtr & rhs) const;	// equality operator
    bool operator!=(const NDimObjPtr & rhs) const;	// not equal to operator
    
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
    
    char * pData;
    unsigned long offset;
    PntType pT;     					// pointer to the actual data
    pointND<NDIM,int> Next;				// lattice extension of the N-dimensional object
    int Ntot;						// total number of lattice points
    
};

#endif

#ifndef NDIMOBJPTR_IMPLEMENTATION
#define NDIMOBJPTR_IMPLEMENTATION

/// standard includes
#include <stdlib.h>
#include <iostream>
#include <ostream>
#include <assert.h>

/// user defined includes
#include "pointND.h"
#include "NDimMemBase.h"
#include "NDimObjPtr.h"

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//                                                         //
//             NDimObjPtr Implementation                   //
//                                                         //
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

// default constructor
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>::NDimObjPtr()
  : pData(NULL),offset(0),pT(),Next(),Ntot(0)
{
  
}

// constructor
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>::NDimObjPtr( char * _pData 
				       ,const unsigned long & _offset
				       ,const PntType & _pT
				       ,const pointND<NDIM,int> & _Next
				       ,const int & _Ntot)
  : pData(_pData),offset(_offset),pT(_pT),Next(_Next),Ntot(_Ntot)
{
  
}

// destructor
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>::~NDimObjPtr()
{
  
}

// assignment operator
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj> & 
    NDimObjPtr<NDIM,NDimObj>::operator=(const NDimObjPtr & rhs)
{
  if ( this != &rhs)
  {
    pData   = rhs.pData;
    offset  = rhs.offset;
    pT	    = rhs.pT;
    Ntot    = rhs.Ntot;
    Next    = rhs.Next;
  }
    
  return *this;
}

// copy constructor
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>::NDimObjPtr(const NDimObjPtr & rhs)
{
  operator=(rhs);
}

// dereference operator
template <short NDIM, typename NDimObj>
  NDimObj   
    NDimObjPtr<NDIM,NDimObj>::operator*() const
{
  #ifdef DEBUG
    std::cout<<"NDimObjPtr::operator*() const with"<<std::endl;
    std::cout<<"pData ="<<(void *)pData<<std::endl;
    std::cout<<"offset="<<offset<<std::endl;
    std::cout<<"pT    ="<<(void *)pT<<std::endl;
    std::cout<<"Next  ="<<Next<<std::endl;
    std::cout<<"is called ..."<<std::endl;
  #endif
  NDimMemBase< NDIM
              ,typename NDimObj::DatValType
              ,NDimObj> BaseObj(pData,offset,pT,Next);
  
  #ifdef DEBUG
    std::cout<<"Returning the result"<<std::endl;
  #endif
  //return (BaseObj);
  return NDimObj(pData,offset,pT,Next);
}

// operator ->
template <short NDIM, typename NDimObj>
  NDimObj 
    NDimObjPtr<NDIM,NDimObj>::operator->() const
{
  #ifdef DEBUG
    std::cout<<"NDimObjPtr::operator->() const is called"<<std::endl;
  #endif
  return operator*();
}

// access operator
template <short NDIM, typename NDimObj>
  NDimObj 
    NDimObjPtr<NDIM,NDimObj>::operator[](const int & idx) const
{
  const NDimObjPtr<NDIM,NDimObj> NDimObjPtrTmp(operator+(idx));
  
  return ( *NDimObjPtrTmp );
}

// add an integer of n elements to the pointer
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj> 
    NDimObjPtr<NDIM,NDimObj>::operator+(const int & n) const
{
  size_t TotSize = (*this)->getObjMemSize();
  
  NDimObjPtr<NDIM,NDimObj> NDimObjPtrTmp(*this);
  
  NDimObjPtrTmp.SetDataMemAddr(pData + n * TotSize);
  
  return NDimObjPtrTmp;
}

// subtract an integer of n elements from the pointer
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj> 
    NDimObjPtr<NDIM,NDimObj>::operator-(const int & n) const
{
  size_t TotSize = (*this)->getObjMemSize();
  
  NDimObjPtr<NDIM,NDimObj> NDimObjPtrTmp(*this);
  
  NDimObjPtrTmp.SetDataMemAddr(pData - n * TotSize);
  
  return NDimObjPtrTmp;
}

// prefix operator: ++NDimObjPtr
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj> & 
    NDimObjPtr<NDIM,NDimObj>::operator++()				
{
  size_t TotSize = (*this)->getObjMemSize();
  
  SetDataMemAddr(pData + TotSize);
  
  return *this;
}

// postfix operator: NDimObjPtr++
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>   
    NDimObjPtr<NDIM,NDimObj>::operator++(int)
{
  NDimObjPtr<NDIM,NDimObj> NDimObjPtrTmp(*this);
  operator++();
  return NDimObjPtrTmp;
}

// equality operator
template <short NDIM, typename NDimObj>
  bool 
    NDimObjPtr<NDIM,NDimObj>::operator==(const NDimObjPtr & rhs) const
{
  #ifdef DEBUG
    std::cout<<"NDimObjPtr<NDIM,NDimObj>::operator=="<<std::endl;
  #endif
  const bool is_equal( ( pData == rhs.pData ) &&
		       ( offset== rhs.offset) &&
		       ( pT    == rhs.pT    ) &&
		       ( Next  == rhs.Next  ) &&
		       ( Ntot  == rhs.Ntot  )
		      );
  #ifdef DEBUG
    std::cout<<"NDimObjPtr<NDIM,NDimObj>::operator== finished. Result ="<<is_equal<<std::endl;
    std::cout<<"NDimObjPtr<NDIM,NDimObj>::operator== finished. Result ="<<is_equal<<std::endl;
  #endif
  return is_equal;
  
}

// "not equal to" operator
template <short NDIM, typename NDimObj>
  bool 
    NDimObjPtr<NDIM,NDimObj>::operator!=(const NDimObjPtr & rhs) const
{
  return ( !( *this == rhs) );
}

template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>::operator void*() const
{
  return ( (void *)pData );
}

template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>::operator char*() const
{
  return ( (char *)pData );
}
// // determines the size in memory of the referenced object
// template <short NDIM, typename NDimObj>   
//   size_t 
//     NDimObjPtr<NDIM,NDimObj>::detMemSize()
// {
//   
// }

template <short NDIM, typename NDimObj>
  inline void 
    NDimObjPtr<NDIM,NDimObj>::SetDataMemAddr(char * _CharPtr)
{
  SetDataMemAddr(_CharPtr, Int2Type<DataTypeDescr<ValType>::is_standard_type>() );
}

// the represented data type is a fundamental type
template <short NDIM, typename NDimObj>
  inline void 
    NDimObjPtr<NDIM,NDimObj>::SetDataMemAddr(char * _CharPtr, Int2Type<true>)
{
  pData =  _CharPtr;
  pT    = (PntType)(pData + offset);
}

// the represented data type is not a fundamental type
template <short NDIM, typename NDimObj>
  inline void 
    NDimObjPtr<NDIM,NDimObj>::SetDataMemAddr(char * _CharPtr, Int2Type<false>)
{
  pData = _CharPtr;
  pT.SetDataMemAddr( _CharPtr + offset );
}

template <short NDIM, typename NDimObj>
  inline NDimObj * 
    NDimObjPtr<NDIM,NDimObj>::getObjPtr()
{
  return new NDimObj(pData,offset,pT,Next);
}

#endif
