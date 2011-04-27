//
// C++ Interface: NDimMemBase
//
// Description: 
//
//
// Author: Daniel Gruenewald, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef NDIMMEMBASE_DEFINITION
#define NDIMMEMBASE_DEFINITION

  /// standard includes
  #include <stdlib.h>
  #include <iostream>
  #include <ostream>
  #include <assert.h>

  /// user defined includes
  #include "pointND.h"
  #include "MemHandler.h"
  #ifndef NDIMOBJPTR_IMPLEMENTATION
    #define NDIMOBJPTR_IMPLEMENTATION
    #define NDIMOBJPTR_IMP_DEF
    #include "NDimObjPtr.h"
  #endif

/**
	@author Daniel Gruenewald

      class implements an N-dimensional block
      of memory
*/


template < short NDIM, typename DataType, class derived>
  class NDimMemBase 
{
  
  public:
  
    typedef typename DataTypeDescr<DataType>::value_type   		DatValType;
    typedef typename DataTypeDescr<DataType>::pointer_type 		DatPntType;
    typedef typename DataTypeDescr<DataType>::reference_type    	DatRefType;
    typedef typename DataTypeDescr<DataType>::reference_return_type 	DatRefRetType;
    
    NDimMemBase();
    NDimMemBase( DatPntType _pFirstElem
    		      ,const pointND<NDIM,int> & _N);
    NDimMemBase( char * const _pData
		         ,const unsigned long _offset
		         ,DatPntType _pT
		         ,const pointND<NDIM,int> &_N);
    ~NDimMemBase();

    // initializes the memory with the given value
    bool InitMem(const DataType &);
    
    // operator new
    //void * operator new(size_t );
    //void * operator new[](size_t );
    // operator delete
    //void operator delete(void *);
    //void operator delete[](void *);
    
    // reference operator
    NDimObjPtr<NDIM,derived> operator&();
    // const reference operator
    const NDimObjPtr<NDIM,derived> operator&() const;
    
    // cast operator
    operator derived();
    // const cast operator
    operator const derived() const;
    
    // dereference operator
    NDimMemBase * operator->();
    
//     // pointwise addition
//     NDimMemBase operator+(const NDimMemBase &) const;
//     // pointwise subtraction
//     NDimMemBase operator-(const NDimMemBase &) const;
//     // pointwise multiplication
//     NDimMemBase operator*(const NDimMemBase &) const;
//     // scalar multiplication
//     NDimMemBase operator*(const DataType &) const;
    
    // assignment operator for const NDimMemBase rhs
    NDimMemBase & operator=(const NDimMemBase &);
    // assignment operator for NDimMemBase rhs
    NDimMemBase & operator=(NDimMemBase &);
    // copy constructor for const NDimMemBase rhs
    NDimMemBase( const NDimMemBase & );
    // copy constructor 
    NDimMemBase( NDimMemBase & );
    // constructor for derived types
    //NDimMemBase(derived &);
    // constructor for constant derived types
    //NDimMemBase(const derived &);
    
    // access operator
    DatRefRetType operator[](const int &);
    DatValType    operator[](const int &) const;
    DatRefRetType operator[](const pointND<NDIM,int> &);
    DatValType    operator[](const pointND<NDIM,int> &) const;
    
    // equality operator
    bool operator==(const NDimMemBase &) const;
    
    // L2 norm
    template <short NDIM_L2,typename DataType_L2, class derived_L2>
        friend DataType_L2 L2Norm(const NDimMemBase<NDIM_L2,DataType_L2,derived_L2> &);
    
    // transpose operator
    // changes the dire
    // index i = {0,1,2} denotes the slowest direction
    //                   of the transposed object
    // index j = {0,1,2} denotes the intermediate direction
    //                   of the transposed object
    // index k = {0,1,2} denotes the fastest direction 
    //                   of the transposed object
    // i,j,k have to be 
    NDimMemBase transpose(const pointND<NDIM,int> &);
    
    // getSubMem returns a submemory region
    // of the given object extended from the
    // lower left corner ll to the upper right
    // corner ur
    NDimMemBase getSubMem(pointND<NDIM,int> ll, pointND<NDIM,int> ur);
    
    // setSubMem sets a submemory region
    // of the given object with the values
    // given by the NDimMemBase object beginning
    // from the lower left corner ll of the 
    // given object
    bool  setSubMem(pointND<NDIM,int> ll, NDimMemBase &);

    // operator <<
    template <short NDIM_OS, typename DataType_OS, class derived_OS>
        friend std::ostream& operator<<( std::ostream &
                                            ,const NDimMemBase<NDIM_OS,DataType_OS,derived_OS> &);
    
    // computes the array index for a given point
    unsigned long point2idx(const pointND<NDIM,int> &) const;

    // computes the point for a given array index
    pointND<NDIM,int> idx2point(const unsigned long &) const;
    
    
    // get routines:
    
    // get the total number of lattice points
    int getNtot() const;
    // get the lattice extension vector
    pointND<NDIM,int> getN() const;
    // get the size of the object in memory
    size_t getObjMemSize() const;
    // get the pointer to the beginning of the raw data
    char * getpData() const;
    // get the pointer to the data
    DatPntType getpT() const;
    // get the offset
    unsigned long getOffset() const;
    
    
    // set routines:
    
    // set the pointer to new address
    void SetDataMemAddr(char * _CharPtr);
    
  protected:
    
//    NDimMemBase( const pointND<NDIM,int> & _N);
    
//     template <bool>
//       DataType & access_op(const int & idx);
//     template <bool>
//       DataType 	 access_op(const int & idx) const;
    
    // request memory
    //char * request_mem(const size_t &);
    void SetpTAddr(char * _CharPtr);
    
  private:
   
    // set the pointer referencing the actual data to the
    // correct address
    void SetpTAddr(char * _CharPtr, Int2Type<true>);
    void SetpTAddr(char * _CharPtr, Int2Type<false>);
    static size_t getMemSize( DatPntType _pT
    		                   ,const unsigned long & _offset
    		                   ,const pointND<NDIM,int> & _N);
    static size_t getMemSize( DatPntType _pT
                              ,const unsigned long & _offset
                              ,const pointND<NDIM,int> & _N
                              ,Int2Type<true>);
    static size_t getMemSize( DatPntType _pT
                              ,const unsigned long & _offset
                              ,const pointND<NDIM,int> & _N
                              ,Int2Type<false>);
    static int getNtotal(const pointND<NDIM,int> & _N);
      
  protected:
    
    // Structure definition
    //NDimMemStrucDef<NDIM,DataType> StrucDef;
    
    // extensions along the several directions
    pointND<NDIM,int> Nlat;
    
    // total number of points 
    int Ntot;
    
    // boolean to indicate whether the current instance 
    // has allocated memory itself or whether it has been
    // given some memory
    //bool selfalloc;
    
    // pointer to the raw data
    //char * pData;
    
    // offset for the real data to the raw data
    unsigned long offset;
    
    // pointer to the data
    DatPntType pT;
    
    MemHandler MemState;
    
};

// template <short NDIM,typename DataType,class derived>
//   struct DataTypeDescr<NDimMemBase<NDIM,DataType,derived> > {
//     typedef NDimMemBase<NDIM,DataType,derived> value_type;
//     typedef NDimObjPtr<NDIM,derived> pointer_type;
//     typedef NDimMemBase<NDIM,DataType,derived> & reference_type;
//     typedef NDimMemBase<NDIM,DataType,derived>   reference_return_type;
//     static const bool is_standard_type = false;
// };

  #ifdef NDIMOBJPTR_IMP_DEF
    #undef NDIMOBJPTR_IMP_DEF
    #undef NDIMOBJPTR_IMPLEMENTATION
    #include "NDimObjPtr.h"
  #endif

#endif

#ifndef NDIMMEMBASE_IMPLEMENTATION
#define NDIMMEMBASE_IMPLEMENTATION

/// standard includes
#include <stdlib.h>
#include <iostream>
#include <ostream>
#include <assert.h>

/// user defined includes
#include "pointND.h"
#include "NDimMemBase.h"

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//                                                         //
//               NDimMemBase Implementation                //
//                                                         //
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

template <short NDIM, typename DataType, class derived> 
    NDimMemBase<NDIM,DataType,derived>::NDimMemBase()
  : MemState(),pT(),offset(0),Nlat(),Ntot(0)
{
  #ifdef DEBUG
	std::cout<<"NDimMemBase<NDIM,DataType,derived>::NDimMemBase() is called"<<std::endl;
  #endif
}

// template <short NDIM, typename DataType, class derived>
//     NDimMemBase<NDIM,DataType,derived>::NDimMemBase( const pointND<NDIM,int> &_N)
//   : pT(NULL),Nlat(_N),selfalloc(true)
// {
// #ifdef DEBUG
//   std::cout<<std::endl;
//   std::cout<<"NDimMemBase<NDIM,DataType,derived>::NDimMemBase( const pointND<NDIM,int> &_N) is called"<<std::endl;
// #endif
//   
//   Ntot = Nlat[0];
//   for(int i = 1; i < NDIM; i++)
//     Ntot*=Nlat[i];
//   
//   // N[i] denote the number of indeces
//   // along the slowest N[0] to the fastest
//   // array direction N[NDIM-1]
//   
//   assert( assign_mem(Ntot) == true );
//   
// #ifdef DEBUG
//   std::cout<<"Nlat = "<<Nlat<<std::endl;
//   std::cout<<"NDimMemBase<NDIM,DataType,derived>::NDimMemBase( const pointND<NDIM,int> &_N) is finished"<<std::endl;
//   std::cout<<std::endl;
// #endif
//   
// }

// Constructor
template <short NDIM, typename DataType, class derived>
    NDimMemBase<NDIM,DataType,derived>::NDimMemBase( DatPntType _pT 
					            ,const pointND<NDIM,int> &_N)
  : MemState((char *)_pT,getMemSize(_pT,0,_N))
   ,offset(0)
   ,pT(_pT)
   ,Nlat(_N)
   ,Ntot(getNtotal(_N))
   { }

template <short NDIM, typename DataType, class derived>
    NDimMemBase<NDIM,DataType,derived>::NDimMemBase( char * const _pData
					    ,const unsigned long _offset 
					    ,DatPntType _pT 
					    ,const pointND<NDIM,int> &_N)
  : MemState(_pData,getMemSize(_pT,_offset,_N))
   ,pT(_pT)
   ,offset(_offset)
   ,Nlat(_N)
   ,Ntot(getNtotal(_N))
{
  SetpTAddr(_pData + _offset);
}

// Destructor
template <short NDIM, typename DataType, class derived>
    NDimMemBase<NDIM,DataType,derived>::~NDimMemBase()
{
#ifdef DEBUG
  std::cout<<"NDimMemBase<NDIM,DataType,derived>::~NDimMemBase()"<<std::endl;
#endif
//  if(selfalloc)
//  {
////     std::cout<<"NDimMemBase<NDIM,DataType,derived>::~NDimMemBase():"<<std::endl;
////     std::cout<<"  selfalloc = "<<selfalloc<<std::endl;
////     std::cout<<"  pData     = "<<(void *)pData<<std::endl;
////     std::cout<<"  pT        = "<<(void *)pT<<std::endl;
////     std::cout<<"  offset    = "<<offset<<std::endl;
////     std::cout<<"  *this     = "<<std::endl;
//    //std::cout<<*this<<std::endl;
//    if(pData != NULL)
//    {
//      //std::cout<<"~NDimMemBase: Deleting address "<<(void *)pData<<std::endl;
//      if(Ntot > 1)
// 	delete[] pData;
//      else
// 	delete pData;
//    }
//  }
  
}

template <short NDIM, typename DataType, class derived>
    bool NDimMemBase<NDIM,DataType,derived>::InitMem(const DataType & _val)
{
  
  for(int i = 0; i < Ntot; i++)
    operator[](i) = _val;
  
  return true;
  
}

// operator new
// template <short NDIM, typename DataType, class derived>
//     void * NDimMemBase<NDIM,DataType,derived>::operator new(size_t _size)
// {
//   //std::cout<<"NDimMemBase<NDIM,DataType,derived>::operator new is called"<<std::endl;
//   return new char[_size];
// }

// operator delete
// template <short NDIM, typename DataType, class derived>
//     void NDimMemBase<NDIM,DataType,derived>::operator delete(void * ptr)
// {
//   //std::cout<<"NDimMemBase<NDIM,DataType,derived>::operator delete is called"<<std::endl;
//   delete[] ptr;
// }

// reference operator
template <short NDIM, typename DataType, class derived>
  NDimObjPtr<NDIM,derived> 
    NDimMemBase<NDIM,DataType,derived>::operator&() 
{
#ifdef DEBUG
  std::cout<<"NDimMemBase<NDIM,DataType,derived>::operator&()"<<std::endl;
#endif
  return NDimObjPtr<NDIM,derived>(MemState.getpData(),offset,pT,Nlat,Ntot);
}

// const reference operator
template <short NDIM, typename DataType, class derived>
  const NDimObjPtr<NDIM,derived > 
    NDimMemBase<NDIM,DataType,derived>::operator&() const
{
  #ifdef DEBUG
    std::cout<<"NDimMemBase<NDIM,DataType,derived>::operator&() const"<<std::endl;
  #endif
  
  return NDimObjPtr<NDIM,derived>(MemState.getpData(),offset,pT,Nlat,Ntot);
}

//dereference operator
template <short NDIM, typename DataType, class derived>
  NDimMemBase<NDIM,DataType,derived> *
    NDimMemBase<NDIM,DataType,derived>::operator->()
{
  #ifdef DEBUG
    std::cout<<"NDimMemBase dereference operator is called"<<std::endl;
  #endif
  return this;
}

// cast operator
template <short NDIM, typename DataType, class derived>
  NDimMemBase<NDIM,DataType,derived>::operator derived()
{
  
  #ifdef DEBUG
    std::cout<<"NDimMemBase: non const cast to derived"<<std::endl;
    std::cout<<"beginning the cast"<<std::endl;
  #endif
  
  derived * pDerived = reinterpret_cast<derived *>(this);

  pDerived->SetDataMemAddr(MemState.getpData());
  
  #ifdef DEBUG
    std::cout<<"NDimMemBase: non const cast to derived finished"<<std::endl;
    std::cout<<"returning result"<<std::endl;
  #endif
  
  return *pDerived;
}

// const cast operator
template <short NDIM, typename DataType, class derived>
  NDimMemBase<NDIM,DataType,derived>::operator const derived() const
{
  #ifdef DEBUG
    std::cout<<"NDimMemBase const cast to derived"<<std::endl;
  #endif  
  
  derived * pDerived = reinterpret_cast<derived *>(const_cast<NDimMemBase *>(this));
  
  pDerived->SetDataMemAddr(MemState.getpData());
  
  const derived * pConstDerived = reinterpret_cast<const derived *>(pDerived);
  
  #ifdef DEBUG
    std::cout<<"NDimMemBase: const cast to derived finished"<<std::endl;
    std::cout<<"returning result"<<std::endl;
  #endif  
  
  return (*pConstDerived);
}

// // pointwise addition
// template <short NDIM, typename DataType, class derived>
//     NDimMemBase<NDIM,DataType,derived> NDimMemBase<NDIM,DataType,derived>::operator+(const NDimMemBase<NDIM,DataType,derived> & _M) const
// {
//   
//   std::cout<<std::endl;
//   std::cout<<"NDimMemBase<NDIM,DataType,derived>::operator+ is called"<<std::endl;
//   
//   // perform check whether dimensions are correct!
//   assert( Nlat == _M.Nlat );
//   
//   NDimMemBase<NDIM,DataType,derived> res(Nlat);
// 
//   for(int i=0;i<Ntot;i++)
//     res.pT[i] = pT[i] + _M.pT[i];
// 
//   std::cout<<"Nlat ="<<res.Nlat<<std::endl;
//   std::cout<<"NDimMemBase<NDIM,DataType,derived>::operator+ is finished"<<std::endl;
//   std::cout<<std::endl;
//   
//   return res;
// }
// 
// // pointwise subtraction
// template <short NDIM, typename DataType, class derived>
//     NDimMemBase<NDIM,DataType,derived> NDimMemBase<NDIM,DataType,derived>::operator-(const NDimMemBase<NDIM,DataType,derived> & _M) const
// {
//   std::cout<<"NDimMemBase<NDIM,DataType,derived>::operator- is called"<<std::endl;
//   
//   // perform check whether dimensions are correct!
//   assert( Nlat == _M.Nlat );
//   
//   NDimMemBase<NDIM,DataType,derived> res(Nlat);
// 
//   for(int i=0;i<Ntot;i++)
//     res.pT[i] = pT[i] - _M.pT[i];
// 
//   return res;
//   
// }
// 
// // pointwise multiplication
// template <short NDIM, typename DataType, class derived>
//     NDimMemBase<NDIM,DataType,derived> NDimMemBase<NDIM,DataType,derived>::operator*(const NDimMemBase<NDIM,DataType,derived> & _M) const
// {
//   std::cout<<"NDimMemBase<NDIM,DataType,derived>::operator* is called"<<std::endl;
//   
//   // perform check whether dimensions are correct!
//   assert( Nlat == _M.Nlat );
//   
//   NDimMemBase<NDIM,DataType,derived> res(Nlat);
// 
//   for(int i=0;i<Ntot;i++)
//     res.pT[i] = pT[i] * _M.pT[i];
// 
//   return res;
// }
// 
// // scalar multiplication
// template <short NDIM, typename DataType, class derived>
//     NDimMemBase<NDIM,DataType,derived> NDimMemBase<NDIM,DataType,derived>::operator*(const DataType & _scal) const
// { 
//   
//   std::cout<<"NDimMemBase<NDIM,DataType,derived>::operator* with scalar is called"<<std::endl;
//   
//   NDimMemBase<NDIM,DataType,derived> res(Nlat);
// 
//   for(int i=0;i<Ntot;i++)
//     res.pT[i] = pT[i] * _scal;
// 
//   return res;
// }

// assignment operator
template <short NDIM, typename DataType, class derived>
    NDimMemBase<NDIM,DataType,derived> & NDimMemBase<NDIM,DataType,derived>::operator=(const NDimMemBase<NDIM,DataType,derived> & _M)
{
  #ifdef DEBUG
    std::cout<<"NDimMemBase<NDIM,DataType,derived>::operator=(const NDimMemBase<NDIM,DataType,derived> & _M)"<<std::endl;
  #endif
  if ( this->operator&() != _M.operator&())
  {
    //char * pData_tmp;
    
    //if( ( pData_tmp = request_mem( _M.getObjMemSize() ) ) != NULL )
    {
      MemState =_M.MemState;
      pT = _M.pT;
      offset = _M.offset;
      Nlat = _M.Nlat;
      Ntot = _M.Ntot;  
      SetpTAddr(MemState.getpData()+offset);
      
    //  memcpy((void *)pData_tmp,(void *)_M.pData,_M.getObjMemSize());
      
//       for( int i = 0;i<Ntot;i++)
//       {
//         operator[](i) = _M[i];
// 	//std::cout<<"_M[i] = "<<_M[i]<<" operator[i] = "<<operator[](i)<<std::endl;
//       }
    }
      
  }
  return *this;

}

// assignment operator
template <short NDIM, typename DataType, class derived>
  NDimMemBase<NDIM,DataType,derived> & NDimMemBase<NDIM,DataType,derived>::operator=( NDimMemBase<NDIM,DataType,derived> & _M)
{
  #ifdef DEBUG
    std::cout<<"NDimMemBase<NDIM,DataType,derived>::operator=(NDimMemBase<NDIM,DataType,derived> & _M)"<<std::endl;
  #endif
  Nlat = _M.Nlat;
  Ntot = _M.Ntot;
  MemState = _M.MemState;
  offset = _M.offset;
  pT = _M.pT;
  SetpTAddr(MemState.getpData()+offset);
  
  return *this;
}

// copy constructor
template <short NDIM, typename DataType, class derived>
    NDimMemBase<NDIM,DataType,derived>::NDimMemBase( const NDimMemBase<NDIM,DataType,derived> & rhs)
    :  MemState(rhs.MemState)
      ,pT(rhs.pT)
      ,offset(rhs.offset)
      ,Nlat(rhs.Nlat)
      ,Ntot(rhs.Ntot)
{
#ifdef DEBUG
	std::cout<<"NDimMemBase<NDIM,DataType,derived>::NDimMemBase( NDimMemBase<NDIM,DataType,derived> &)"<<std::endl;
#endif
	SetpTAddr(MemState.getpData() + rhs.offset);
}

template <short NDIM, typename DataType, class derived>
  NDimMemBase<NDIM,DataType,derived>::NDimMemBase(NDimMemBase<NDIM,DataType,derived> & rhs)
:  MemState(rhs.MemState)
  ,pT(rhs.pT)
  ,offset(rhs.offset)
  ,Nlat(rhs.Nlat)
  ,Ntot(rhs.Ntot)
{
  #ifdef DEBUG
    std::cout<<"NDimMemBase<NDIM,DataType,derived>::NDimMemBase( NDimMemBase<NDIM,DataType,derived> &)"<<std::endl;
  #endif
 SetpTAddr(MemState.getpData() + rhs.offset);
}

// // copy constructor
// template <short NDIM, typename DataType, class derived>
//     NDimMemBase<NDIM,DataType,derived>::NDimMemBase( const derived & rhs)
//   : pData(rhs.getpData())
//    ,offset(rhs.getOffset())
//    ,pT(rhs.getpT())
//    ,Nlat(rhs.getN())
//    ,selfalloc(false)
// {
//   #ifdef DEBUG
//   std::cout<<"NDimMemBase<NDIM,DataType,derived>::NDimMemBase( const derived &)"<<std::endl;
//   #endif
// }
// 
// // copy constructor
// template <short NDIM, typename DataType, class derived>
//     NDimMemBase<NDIM,DataType,derived>::NDimMemBase( derived & rhs)
//   : pData(rhs.getpData())
//    ,offset(rhs.getOffset())
//    ,pT(rhs.getpT())
//    ,Nlat(rhs.getN())
//    ,selfalloc(false)
// {
//   #ifdef DEBUG
//   std::cout<<"NDimMemBase<NDIM,DataType,derived>::NDimMemBase( derived &)"<<std::endl;
//   #endif
// }


// template <short NDIM, typename DataType, class derived>
//   template <bool isfund >
//     DataType & 
//       NDimMemBase<NDIM,DataType,derived>::access_op(const int & idx)
// {
//   return pT[idx];
// }
// 
// template <short NDIM, typename DataType, class derived>
//   template <bool isfund >
//     DataType  
//       NDimMemBase<NDIM,DataType,derived>::access_op(const int & idx) const
// {
//   return pT[idx];
// }

// access operator
template <short NDIM, typename DataType, class derived>
  typename NDimMemBase<NDIM,DataType,derived>::DatRefRetType
    NDimMemBase<NDIM,DataType,derived>::operator[](const int & idx)
{
  return (pT[idx]);
}
    
template <short NDIM, typename DataType, class derived>
  typename NDimMemBase<NDIM,DataType,derived>::DatValType   
    NDimMemBase<NDIM,DataType,derived>::operator[](const int & idx) const
{
  return (pT[idx]);
}
    
template <short NDIM, typename DataType, class derived>
  typename NDimMemBase<NDIM,DataType,derived>::DatRefRetType
    NDimMemBase<NDIM,DataType,derived>::operator[](const pointND<NDIM,int> & pnt)
{
  return (operator[](point2idx(pnt)));
}
    
template <short NDIM, typename DataType, class derived>
  typename NDimMemBase<NDIM,DataType,derived>::DatValType     
    NDimMemBase<NDIM,DataType,derived>::operator[](const pointND<NDIM,int> & pnt) const
{
  return (operator[](point2idx(pnt)));
}

// equality operator
template <short NDIM, typename DataType, class derived>
    bool NDimMemBase<NDIM,DataType,derived>::operator==(const NDimMemBase<NDIM,DataType,derived> & rhs) const
{
  return ( ( Nlat == rhs.Nlat ) ? true : false );
}

// L2 norm
template <short NDIM, typename DataType, class derived>
    DataType L2Norm(const NDimMemBase<NDIM,DataType,derived> & _M)
{
  DataType res = _M[0]*_M[0];
  for(int i=1;i<_M.Ntot;i++)
  {
    res+=_M[i]*_M[i];
  }
  return res;
}

template <short NDIM, typename DataType, class derived>
    NDimMemBase<NDIM,DataType,derived> NDimMemBase<NDIM,DataType,derived>::transpose(const pointND<NDIM,int> & Perm) 
{
  // Input: Perm
  //        contains the mapping of the directions
  //        Perm[0] <- contains the direction which is 
  //                   the slowest in the result
  //        Perm[1] <- contains the direction which is
  //                   the second slowest in the result
  //           .
  //           .
  //           .
  //        Perm[NDIM-1] <- contains the direction which
  //                   is the fastes in the result
  
  
  // Check the parameter validity here
  {
    // First, check whether the permutation vector is
    // in a valid range, i.e. that each component is
    // greater equal to zero and less than NDIM
    for(int i = 0; i < NDIM; i++)
      assert ( Perm[i] >= 0 && Perm[i] < NDIM );
    
    // Second, check that the components of the 
    // permutation vector are pointwise different
    for(int i = 0; i < NDIM; i++)
    {
      for(int j = (i + 1) ; j<NDIM; j++)
      {
        assert ( Perm[i] != Perm[j] );
      }
    }
  }
  
  pointND<NDIM,int> Nlatres;

  for(int i = 0; i < NDIM; i++)
  {
    Nlatres[i] = Nlat[Perm[i]];
  }
  
  
  NDimMemBase<NDIM,DataType,derived> res(pT,Nlatres);
  // request the memory
  char * pDataTmp = new char[res.getObjMemSize()];
  res.SetDataMemAddr(pDataTmp);
  res.MemState.SetSelfAlloc();
  
  for(int myidx = 0; myidx < Ntot; myidx++)
  {
    int residx;
    { // compute the mapping from myindex
      const pointND<NDIM,int> mypnt = idx2point(myidx);
            pointND<NDIM,int> respnt;
      for(int i = 0; i < NDIM; i++)
        respnt[i] = mypnt[Perm[i]];
      residx = res.point2idx(respnt);
    }
    
    res[residx] = operator[](myidx);
  } 
  
  return res;
}

template <short NDIM, typename DataType, class derived>
    NDimMemBase<NDIM,DataType,derived> NDimMemBase<NDIM,DataType,derived>::getSubMem(pointND<NDIM,int> ll, pointND<NDIM,int> ur)
{
  
  // ll: lower left corner of the ND volume
  //     to be cut
  // ur: upper right corner of the ND volume
  //     to be cut
  
  // assert that the volume to be extracted
  // is part of the volume
  
  for(int i = 0; i < NDIM; i++)
    assert( ll[i] >= 0 && ll[i] < Nlat[i] );
  
  for(int i = 0; i < NDIM; i++)
    assert( ur[i] >= 0 && ur[i] < Nlat[i] );
  
  for(int i = 0; i < NDIM; i++)
    assert( ll[i] <= ur[i] );
  
  
  // Compute the resulting lattice dimensions
  pointND<NDIM,int> NlatRes;
  for(int i = 0; i < NDIM; i++)
    NlatRes[i] = ur[i] - ll[i] + 1;
  
  // generate result vector of correct 
  // dimensions
  // request the memory
  DatPntType pT_help = pT;
  NDimMemBase res(pT_help,NlatRes);
  char * pDataTmp = new char[res.getObjMemSize()];
  res.SetDataMemAddr(pDataTmp);
  res.MemState.SetSelfAlloc();
  
  for(int residx = 0; residx < res.Ntot; residx++)
  {
    const pointND<NDIM,int> respnt = res.idx2point(residx);
    const pointND<NDIM,int>  mypnt = ll + respnt;
    const int myidx = point2idx(mypnt);
    res[residx] = operator[](myidx);
  } 
  
  return res;
}

template <short NDIM, typename DataType, class derived>
    bool NDimMemBase<NDIM,DataType,derived>::setSubMem(pointND<NDIM,int> ll, NDimMemBase<NDIM,DataType,derived> & _SubMem)
{
  
  // ll    : lower left corner of the 3D volume
  //         at which the subvolume is to be inserted
  // SubMem: SubMemory which is to be inserted
  
  // assert that the volume can be set 
  // as part of the volume
  
  for(int i = 0; i < NDIM; i++)
    assert( ll[i] >= 0 && ll[i] < Nlat[i] );
  
  pointND<NDIM,int> ur;
  for(int i = 0; i < NDIM; i++)
  { 
    ur[i] = ll[i] + _SubMem.Nlat[i] - 1;
    assert( ur[i] >= 0 && ur[i] < Nlat[i] );
  }
  
  for(int SubMemidx = 0; SubMemidx < _SubMem.Ntot;SubMemidx++)
  {
    const pointND<NDIM,int> SubMempnt = _SubMem.idx2point(SubMemidx);
    const pointND<NDIM,int>     mypnt = ll + SubMempnt;
    const int                   myidx = point2idx(mypnt);
    operator[](myidx) = _SubMem[SubMemidx];
    std::cout<<"SubMem[idx] = "<<_SubMem[SubMemidx]<<std::endl;
  }
  
  return true;
}

// outstream operator
template <short NDIM, typename DataType, class derived>
    std::ostream& operator<<(std::ostream & os, const NDimMemBase<NDIM,DataType,derived> & _M)
{ 
  {
    pointND<NDIM,int> ll;
    for(int i = 0; i < NDIM; i++)
      ll[i] = 0;
    os<<std::endl<<"NDimMemBase: lower left corner : "<<ll<<std::endl;
  }
  {
    pointND<NDIM,int> ur;
    for(int i = 0; i < NDIM; i++)
      ur[i] = _M.Nlat[i] - 1;
    os<<"NDimMemBase: upper right corner: "<<ur<<std::endl;
  }
  os<<std::endl;
  
  int idx = 0;
  
  #ifdef DEBUG
    std::cout<<"&pData = "<<(void *)_M.getpData()<<std::endl;
    std::cout<<"offset = "<<_M.offset<<std::endl;
    std::cout<<"&pT    = "<<(void *)&_M.pT[0]<<std::endl;
    std::cout<<"pT[0]  = "<<_M.pT[0]<<std::endl;
  #endif
  for(int myidx = 0; myidx < _M.Ntot; myidx ++)
  {
    os<<"NDimMemBase:           value at: "<<_M.idx2point(myidx)<<" : "
        <<_M[myidx]<<std::endl;
  }
  
  return os;
}

template <short NDIM, typename DataType, class derived>
    unsigned long NDimMemBase<NDIM,DataType,derived>::point2idx(const pointND<NDIM,int> & pnt) const
{
  /*( ( pnt[0] * N[1] + pnt[1] ) * N[2] + pnt[2] )*/
  
  assert(NDIM > 0);
  
  unsigned long res = pnt[0];
  
  if ( NDIM > 1 )
  {
    for(int i = 1; i < NDIM; i++)
    {
      res= res * Nlat[i] + pnt[i];
    }
  }
  return res;
}

template <short NDIM, typename DataType, class derived>
    pointND<NDIM,int> NDimMemBase<NDIM,DataType,derived>::idx2point(const unsigned long & idx) const
{
  pointND<NDIM,int> res;
  
  unsigned long mul = Ntot;   // Number of lattice points
  unsigned long   k = idx;
  for(int i = 0; i < NDIM; i++)
  {
    mul   /= Nlat[i];
    res[i] = k / mul;
    k     -= res[i] * mul;
  }
  
  return res;
}

template <short NDIM, typename DataType, class derived>
    int  NDimMemBase<NDIM,DataType,derived>::getNtot() const
{
  return Ntot;
}
    
template <short NDIM, typename DataType, class derived>
    pointND<NDIM,int>  NDimMemBase<NDIM,DataType,derived>::getN() const
{
  return Nlat;
}

//template <short NDIM, typename DataType, class derived>
//    char * NDimMemBase<NDIM,DataType,derived>::request_mem(const size_t & _memsize)
//{
//
//  if(selfalloc)
//  {
//
//    if( pData != NULL)
//    {
//      #ifdef DEBUG
//        std::cout<<"request_mem: Deleting address "<<(void *)pData<<std::endl;
//      #endif
//      delete[] pData;
//      pData = NULL;
//    }
//#ifdef SHOWMEMALLOC
//    std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;
//    std::cout<<"!!! NDimMemBase: Allocating memory !!!"<<std::endl;
//    std::cout<<"!!! Memsize = "<<_memsize<<"        !!!"<<std::endl;
//    std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl<<std::endl;
//#endif
//    pData =  new char[_memsize];
////     std::cout<<"request_mem: Requested address "<<(void *)pData
////              <<" size = "<<_memsize<<std::endl;
//
//  } else
//  {
//    assert (_memsize <= getObjMemSize() );
//  }
//
////   std::cout<<"request_mem finished ("<<_memsize<<" Byte)"<<std::endl;
//  return ( pData );
//
//}

template <short NDIM, typename DataType, class derived>
inline void 
  NDimMemBase<NDIM,DataType,derived>::SetDataMemAddr(char * _CharPtr)
{
  #ifdef DEBUG
    std::cout<<"NDimMemBase::SetDataMemAddr is called"<<std::endl;
  #endif
  const size_t DatSize = getObjMemSize();
  MemState.SetpData(_CharPtr,DatSize);
  SetpTAddr(_CharPtr + offset);
}

template <short NDIM, typename DataType, class derived>
inline void
  NDimMemBase<NDIM,DataType,derived>::SetpTAddr(char * _CharPtr)
{
  #ifdef DEBUG
    std::cout<<"NDimMemBase::SetDataMemAddr is called"<<std::endl;
  #endif
  SetpTAddr(_CharPtr, Int2Type<DataTypeDescr<DatValType>::is_standard_type>() );
}

// the represented data type is a fundamental type
template <short NDIM, typename DataType, class derived>
  inline void 
    NDimMemBase<NDIM,DataType,derived>::SetpTAddr(char * _CharPtr, Int2Type<true>)
{
#ifdef DEBUG
  std::cout<<"NDimMemBase::Setting pData to "<<(void *)_CharPtr<<std::endl;
  std::cout<<"NDimMemBase::Setting pT    to "<<(void *)(DatPntType)( _CharPtr + offset )<<std::endl;
#endif
  pT = (DatPntType)( _CharPtr );
  
}

// the represented data type is not a fundamental type
template <short NDIM, typename DataType, class derived>
  inline void 
    NDimMemBase<NDIM,DataType,derived>::SetpTAddr(char * _CharPtr, Int2Type<false>)
{
  pT.SetDataMemAddr( _CharPtr );
}

template <short NDIM, typename DataType, class derived>
  inline size_t 
    NDimMemBase<NDIM,DataType,derived>::getObjMemSize() const
{
  return getMemSize(pT,offset,Nlat);
}

template <short NDIM, typename DataType, class derived>
  inline size_t
    NDimMemBase<NDIM,DataType,derived>::getMemSize( DatPntType _pT
    		                                             ,const unsigned long & _offset
    		                                             ,const pointND<NDIM,int> & _N)
{
  return getMemSize( _pT
		             ,_offset
		             ,_N
		             ,Int2Type<DataTypeDescr<DatValType>::is_standard_type>() );
}

// the represented data type is a fundamental type
template <short NDIM, typename DataType, class derived>
  inline size_t 
    NDimMemBase<NDIM,DataType,derived>::getMemSize( DatPntType _pT
    		                                                ,const unsigned long & _offset
    		                                                ,const pointND<NDIM,int> & _N
    		                                                ,Int2Type<true>)
{
  return ( getNtotal(_N) * sizeof(DatValType) + _offset );
}

// the represented data type is not a fundamental type
template <short NDIM, typename DataType, class derived>
  inline size_t
    NDimMemBase<NDIM,DataType,derived>::getMemSize( DatPntType _pT
                                                      ,const unsigned long & _offset
                                                      ,const pointND<NDIM,int> & _N
    		                                          ,Int2Type<false>)
{
  return ( getNtotal(_N) * _pT->getObjMemSize() + _offset );
}

template <short NDIM, typename DataType, class derived>
  inline int
    NDimMemBase<NDIM,DataType,derived>::getNtotal(const pointND<NDIM,int> & _N)
{
	int Ntotal = _N[0];

	for(int i = 1; i < NDIM; i++)
		Ntotal*=_N[i];

	return Ntotal;
}

template <short NDIM, typename DataType, class derived>
  inline char *
    NDimMemBase<NDIM,DataType,derived>::getpData() const
{
  return MemState.getpData();
}

template <short NDIM, typename DataType, class derived>
  inline typename NDimMemBase<NDIM,DataType,derived>::DatPntType
    NDimMemBase<NDIM,DataType,derived>::getpT() const
{
  return pT;
}

template <short NDIM, typename DataType, class derived>
  unsigned long
    NDimMemBase<NDIM,DataType,derived>::getOffset() const
{
  return offset;
}
#endif
