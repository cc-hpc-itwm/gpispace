//
// C++ Interface: NDimMemPtr
//
// Description: 
//
//
// Author: Daniel Gruenewald, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef NDIMMEMPTR
#define NDIMMEMPTR

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

\brief class implements a pointer class to NDimMem(Base) and NDimData(Base)
\details
\pre
*/

template <short NDIM, class NDimObj>
class NDimMemPtr {
  
  typedef typename NDimObj::DatValType ValType;
  typedef typename NDimObj::DatPntType PntType;
  typedef typename NDimObj::DatRefType RefType;
  
  /// Methods
  
  public: 
    
    NDimMemPtr();					// Default constructor
    NDimMemPtr( char * _pData 
               ,const unsigned long & _offset
	       ,const PntType & _pT
               ,const pointND<NDIM,int> & _Next
	       ,const int & _Ntot);			// constructor
    ~NDimMemPtr();					// destructor
    
    NDimMemPtr & operator=(const NDimMemPtr & rhs);	// assignment operator
    NDimMemPtr(const NDimMemPtr & rhs);			// copy constructor
    
    NDimObj operator*() const;				// derefence operator
    NDimObj operator->() const;				// operator ->
    
    NDimObj operator[](const int & idx) const;	        // access operator
    NDimMemPtr operator+(const int & n) const;		// add an integer of n elements to the pointer
    NDimMemPtr operator-(const int & n) const;		// subtract an integer of n elements from the pointer
    
    NDimMemPtr & operator++();				// prefix operator
    NDimMemPtr   operator++(int);			// postfix operator
    
    bool operator==(const NDimMemPtr & rhs) const;	// equality operator
    bool operator!=(const NDimMemPtr & rhs) const;	// not equal to operator
    
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

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//                                                         //
//             NDimMemPtr Implementation                   //
//                                                         //
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

// default constructor
template <short NDIM, typename NDimObj>
  NDimMemPtr<NDIM,NDimObj>::NDimMemPtr()
  : pData(NULL),offset(0),pT(),Next(),Ntot(0)
{
  
}

// constructor
template <short NDIM, typename NDimObj>
  NDimMemPtr<NDIM,NDimObj>::NDimMemPtr( char * _pData 
				       ,const unsigned long & _offset
				       ,const PntType & _pT
				       ,const pointND<NDIM,int> & _Next
				       ,const int & _Ntot)
  : pData(_pData),offset(_offset),pT(_pT),Next(_Next),Ntot(_Ntot)
{
  
}

// destructor
template <short NDIM, typename NDimObj>
  NDimMemPtr<NDIM,NDimObj>::~NDimMemPtr()
{
  
}

// assignment operator
template <short NDIM, typename NDimObj>
  NDimMemPtr<NDIM,NDimObj> & 
    NDimMemPtr<NDIM,NDimObj>::operator=(const NDimMemPtr & rhs)
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
  NDimMemPtr<NDIM,NDimObj>::NDimMemPtr(const NDimMemPtr & rhs)
{
  operator=(rhs);
}

// dereference operator
template <short NDIM, typename NDimObj>
  NDimObj  
    NDimMemPtr<NDIM,NDimObj>::operator*() const
{
  return NDimObj(pData,offset,pT,Next);
}

// operator ->
template <short NDIM, typename NDimObj>
  NDimObj 
    NDimMemPtr<NDIM,NDimObj>::operator->() const
{
  return operator*();
}

// access operator
template <short NDIM, typename NDimObj>
  NDimObj 
    NDimMemPtr<NDIM,NDimObj>::operator[](const int & idx) const
{
  const NDimMemPtr<NDIM,NDimObj> NDimMemPtrTmp(operator+(idx));
  
  return ( *NDimMemPtrTmp );
}

// add an integer of n elements to the pointer
template <short NDIM, typename NDimObj>
  NDimMemPtr<NDIM,NDimObj> 
    NDimMemPtr<NDIM,NDimObj>::operator+(const int & n) const
{
  size_t TotSize = (*this)->getObjMemSize();
  
  NDimMemPtr<NDIM,NDimObj> NDimMemPtrTmp(*this);
  
  NDimMemPtrTmp.SetDataMemAddr(pData + n * TotSize);
  
  return NDimMemPtrTmp;
}

// subtract an integer of n elements from the pointer
template <short NDIM, typename NDimObj>
  NDimMemPtr<NDIM,NDimObj> 
    NDimMemPtr<NDIM,NDimObj>::operator-(const int & n) const
{
  size_t TotSize = (*this)->getObjMemSize();
  
  NDimMemPtr<NDIM,NDimObj> NDimMemPtrTmp(*this);
  
  NDimMemPtrTmp.SetDataMemAddr(pData - n * TotSize);
  
  return NDimMemPtrTmp;
}

// prefix operator: ++NDimMemPtr
template <short NDIM, typename NDimObj>
  NDimMemPtr<NDIM,NDimObj> & 
    NDimMemPtr<NDIM,NDimObj>::operator++()				
{
  size_t TotSize = (*this)->getObjMemSize();
  
  SetDataMemAddr(pData + TotSize);
  
  return *this;
}

// postfix operator: NDimMemPtr++
template <short NDIM, typename NDimObj>
  NDimMemPtr<NDIM,NDimObj>   
    NDimMemPtr<NDIM,NDimObj>::operator++(int)
{
  NDimMemPtr<NDIM,NDimObj> NDimMemPtrTmp(*this);
  operator++();
  return NDimMemPtrTmp;
}

// equality operator
template <short NDIM, typename NDimObj>
  bool 
    NDimMemPtr<NDIM,NDimObj>::operator==(const NDimMemPtr & rhs) const
{
  const bool is_equal( ( pData == rhs.pData ) &&
		       ( offset== rhs.offset) &&
		       ( pT    == rhs.pT    ) &&
		       ( Next  == rhs.Next  ) &&
		       ( Ntot  == rhs.Ntot  )
		      );
  
  return is_equal;
  
}

// "not equal to" operator
template <short NDIM, typename NDimObj>
  bool 
    NDimMemPtr<NDIM,NDimObj>::operator!=(const NDimMemPtr & rhs) const
{
  return ( !( *this == rhs) );
}

template <short NDIM, typename NDimObj>
  NDimMemPtr<NDIM,NDimObj>::operator void*() const
{
  return ( (void *)pData );
}

template <short NDIM, typename NDimObj>
  NDimMemPtr<NDIM,NDimObj>::operator char*() const
{
  return ( (char *)pData );
}
// // determines the size in memory of the referenced object
// template <short NDIM, typename NDimObj>   
//   size_t 
//     NDimMemPtr<NDIM,NDimObj>::detMemSize()
// {
//   
// }

template <short NDIM, typename NDimObj>
  inline void 
    NDimMemPtr<NDIM,NDimObj>::SetDataMemAddr(char * _CharPtr)
{
  SetDataMemAddr(_CharPtr, Int2Type<DataTypeDescr<ValType>::is_standard_type>() );
}

// the represented data type is a fundamental type
template <short NDIM, typename NDimObj>
  inline void 
    NDimMemPtr<NDIM,NDimObj>::SetDataMemAddr(char * _CharPtr, Int2Type<true>)
{
  pData =  _CharPtr;
  pT    = (PntType)(pData + offset);
}

// the represented data type is not a fundamental type
template <short NDIM, typename NDimObj>
  inline void 
    NDimMemPtr<NDIM,NDimObj>::SetDataMemAddr(char * _CharPtr, Int2Type<false>)
{
  pData = _CharPtr;
  pT.SetDataMemAddr( _CharPtr + offset );
}

template <short NDIM, typename NDimObj>
  inline NDimObj * 
    NDimMemPtr<NDIM,NDimObj>::getObjPtr()
{
  return new NDimObj(pData,offset,pT,Next);
}

#endif