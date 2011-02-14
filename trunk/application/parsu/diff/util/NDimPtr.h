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

#ifndef NDIMOBJPTR
#define NDIMOBJPTR

/// standard includes
#include <stdlib.h>
#include <iostream>
#include <ostream>
#include <assert.h>
#include <stdexcept>

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

/**
@author Daniel Gruenewald

\brief class implements a pointer class to NDimMem(Base) and NDimData(Base)
\details
\pre
*/

template <short NDIM, class NDimObj>
class NDimObjPtr {
  
  typedef typename DataTypeDescr<NDimObj>::value_type     ValType;
  typedef typename DataTypeDescr<NDimObj>::pointer_type   PntType;
  typedef typename DataTypeDescr<NDimObj>::reference_type RefType;
  
  /// Methods
  
  public: 
    
    NDimObjPtr();					// Default constructor
    NDimObjPtr(NDimObj * _pNDimObj);			// constructor
    ~NDimObjPtr();					// destructor
    
    NDimObjPtr & operator=(NDimObjPtr & rhs);	        // assignment operator
    NDimObjPtr(NDimObjPtr & rhs);
    NDimObjPtr & operator=(const NDimObjPtr & rhs);	// assignment operator
    NDimObjPtr(const NDimObjPtr & rhs);			// copy constructor
    
    RefType   operator*() const;				// derefence operator
    ValType * operator->() const; 	         		// operator ->
    
    ValType operator[](const int & idx) const;		// access operator
    RefType operator[](const int & idx);		// access operator
    
    NDimObjPtr operator+(const int & n) const;		// add an integer of n elements to the
                                                        // pointer
    NDimObjPtr operator-(const int & n) const;		// subtract an integer of n elements from t 
                                                        // the pointer
    
    NDimObjPtr & operator++();				// prefix add operator
    NDimObjPtr   operator++(int);			// postfix add operator
    
    NDimObjPtr & operator--();				// prefix add operator
    NDimObjPtr & operator--(int);			// postfix add operator
    
    bool operator==(const NDimObjPtr & rhs) const;	// equality operator
    bool operator!=(const NDimObjPtr & rhs) const;	// not equal to operator
    
    operator void*() const;				// cast operator to void * 
							// Required in order to compare to NULL
    operator char*() const;				// cast operator to char *
    
    void SetDataMemAddr(char * _CharPtr);
   
  private:
    
    // copy the pointer without increasing the reference number
    NDimObjPtr copyPtr() const;
   
  /// Member variables:
    
  private:
    
    class PointerHolder {
      
      public:
	
	PointerHolder() : pNDimObj(NULL)
			 ,allocObjPtr(false)
			 ,refCnt(0)
	{}
	
	PointerHolder(NDimObj * _pNDimObj) : pNDimObj(_pNDimObj)
                                            ,allocObjPtr(false)
					    ,refCnt(1)
	{}
	
	~PointerHolder() 
	{
	  if ( refCnt == 0 && allocObjPtr)
	  {
	    if ( pNDimObj != NULL )
	    {
	      //delete pNDimObj;
	    }
	  }
	}
	
	long IncRefCnt()
	{
	  return (++refCnt);
	}
	
	long DecRefCnt()
	{
	  if ( (--refCnt) < 0 )
	  {
	    throw std::runtime_error("ref cnt < 0");
	  }
	  return (refCnt);
	}
      
      public:
	
	NDimObj * pNDimObj;					// Pointer to the NDimObj
	bool allocObjPtr;					// Boolean whether the object
								// was allocated or not
	long refCnt;				                // Reference counting variable
	
    };
    
    PointerHolder * pMetaInfo;
    
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//                                                         //
//             NDimObjPtr Implementation                   //
//                                                         //
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

// default constructor: OK
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>::NDimObjPtr()
  : pMetaInfo(NULL)
{ }

// constructor: OK
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>::NDimObjPtr(NDimObj * _pNDimObj)
  : pMetaInfo(new NDimObjPtr::PointerHolder(_pNDimObj) )
{ }

// destructor: OK
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>::~NDimObjPtr()
{
  std::cout<<"~NDimObjPtr()"<<std::endl;
  if ( (pMetaInfo->DecRefCnt()) == 0)
  {
    delete pMetaInfo;
  }
  
}

// assignment operator
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj> & 
    NDimObjPtr<NDIM,NDimObj>::operator=(NDimObjPtr & rhs)
{
  
  std::cout<<"NDimObjPtr<NDIM,NDimObj>::operator=(NDimObjPtr & rhs)"<<std::endl;
  if ( this != &rhs)
  {
    
    pMetaInfo = rhs.pMetaInfo;
    pMetaInfo->IncRefCnt();
    
  }
  
  return *this;
  
}

// copy constructor: OK
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>::NDimObjPtr(NDimObjPtr & rhs)
  : pMetaInfo(NULL)
{
  operator=(rhs);
}

// assignment operator
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj> & 
    NDimObjPtr<NDIM,NDimObj>::operator=(const NDimObjPtr & rhs)
{
  
  std::cout<<"NDimObjPtr<NDIM,NDimObj>::operator=(const NDimObjPtr & rhs)"<<std::endl;
  if ( this != &rhs)
  {
    PointerHolder * pMetaInfoOld = pMetaInfo;
    
    pMetaInfo = new NDimObjPtr::PointerHolder( rhs.pMetaInfo->pNDimObj->duplicateObj() );
    pMetaInfo->allocObjPtr = true;
    
    if(pMetaInfoOld->DecRefCnt() == 0)
    {
      delete pMetaInfoOld;
    }
    
  }
  
  return *this;
}

// copy constructor: OK
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>::NDimObjPtr(const NDimObjPtr & rhs)
  : pMetaInfo(NULL)
{
  operator=(rhs);
}

// dereference operator: OK !
template <short NDIM, typename NDimObj>
  typename NDimObjPtr<NDIM,NDimObj>::RefType  
    NDimObjPtr<NDIM,NDimObj>::operator*() const
{
  return *(pMetaInfo->pNDimObj);
}

// operator ->: OK
template <short NDIM, typename NDimObj>
  typename NDimObjPtr<NDIM,NDimObj>::ValType *  
    NDimObjPtr<NDIM,NDimObj>::operator->() const
{
  return pMetaInfo->pNDimObj;
}

// access operator with vaalue return type: OK
template <short NDIM, typename NDimObj>
  typename NDimObjPtr<NDIM,NDimObj>::ValType
    NDimObjPtr<NDIM,NDimObj>::operator[](const int & idx) const
{
  return  ( *( (*this) + idx ) );
}

// access operator with reference return type: OK
template <short NDIM, typename NDimObj>
  typename NDimObjPtr<NDIM,NDimObj>::RefType
    NDimObjPtr<NDIM,NDimObj>::operator[](const int & idx) 
{
  return  ( *( (*this) + idx ) );
}

// add an integer of n elements to the pointer: OK
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj> 
    NDimObjPtr<NDIM,NDimObj>::operator+(const int & n) const
{
  
  NDimObjPtr<NDIM,NDimObj> Res(this->copyPtr());
  
  // Set the object data address to the appropriate value
  {
    const NDimObj * pNDimObj = pMetaInfo->pNDimObj;
    // obtain memory size required by the object
    size_t TotMemSize = pNDimObj->getObjMemSize();
    char * pObjData   = pNDimObj->getpData();
    Res->SetDataMemAddr(pObjData + n * TotMemSize);
  }
  
  return Res;;
  
}

// subtract an integer of n elements from the pointer: OK
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj> 
    NDimObjPtr<NDIM,NDimObj>::operator-(const int & n) const
{
  
  NDimObjPtr<NDIM,NDimObj> Res(this->copyPtr());
  
  // Set the object data address to the appropriate value
  {
    const NDimObj * pNDimObj = pMetaInfo->pNDimObj;
    // obtain memory size required by the object
    const size_t TotMemSize = pNDimObj->getObjMemSize();
    char * pObjData   = pNDimObj->getpData();
    Res->SetDataMemAddr(pObjData - n * TotMemSize);
  }
  
  return Res;
  
}

// prefix operator: ++NDimObjPtr: OK
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj> & 
    NDimObjPtr<NDIM,NDimObj>::operator++()				
{
  const NDimObj * pNDimObj = pMetaInfo->pNDimObj;
  const size_t TotMemSize = pNDimObj->getObjMemSize();
  char * pObjData   = pNDimObj->getpData();
  
  pNDimObj->SetDataMemAddr(pObjData + TotMemSize);
  
  return *this;
}

// postfix operator: NDimObjPtr++: OK
template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>   
    NDimObjPtr<NDIM,NDimObj>::operator++(int)
{
  
  NDimObjPtr<NDIM,NDimObj> Res(*this);
  operator++()
  return Res;
  
}

// equality operator: OK
template <short NDIM, typename NDimObj>
  bool 
    NDimObjPtr<NDIM,NDimObj>::operator==(const NDimObjPtr & rhs) const
{
  
  return ( pMetaInfo->pNDimObj == rhs.pMetaInfo->pNDimObj );
  
}

// "not equal to" operator : OK
template <short NDIM, typename NDimObj>
  bool 
    NDimObjPtr<NDIM,NDimObj>::operator!=(const NDimObjPtr & rhs) const
{
  return ( !( *this == rhs) );
}

template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>::operator void*() const
{
  return ( (void *)(pMetaInfo->pNDimObj->getpData() ) );
}

template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>::operator char*() const
{
  return ( (char *)(pMetaInfo->pNDimObj->getpData() ) );
}

template <short NDIM, typename NDimObj>
  void 
    NDimObjPtr<NDIM,NDimObj>::SetDataMemAddr(char * _CharPtr)
{
  pMetaInfo->pNDimObj->SetDataMemAddr(_CharPtr);
}

/// private member functions

template <short NDIM, typename NDimObj>
  NDimObjPtr<NDIM,NDimObj>  
    NDimObjPtr<NDIM,NDimObj>::copyPtr() const
{
  
  NDimObjPtr Res( pMetaInfo->pNDimObj->duplicateObj() );
  
  Res.pMetaInfo->allocObjPtr = true;
  
  return Res;
  
}

#endif