//
// C++ Interface: NDimMem
//
// Description: Adds zero copy / single loop functionality to
//		the NDimMemBase class
//
//
// Author: Daniel Gruenewald, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef NDIMMEM_H
#define NDIMMEM_H

/// standard includes
#include <sstream>

/// user defined includes
#include "NDimMemBase.h"
#include "base.h"
#include "operator_triple.h"

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//                                                         //
//                  NDimMem Declaration                    //
//                                                         //
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

template <short NDIM = 3, typename DataType = float >
  class NDimMem : public NDimMemBase< NDIM
		 		     ,DataType
				     ,NDimMem< NDIM
					      ,DataType> >
		 ,public Base_Obj   < NDimMem< NDIM
				     ,DataType> 
				     ,DataType >
{

  typedef NDimMemBase<NDIM,DataType,NDimMem<NDIM,DataType> > base;
  
  public:
    
    // default constructor
    NDimMem();
    
    // standard constructor
    NDimMem( typename base::DatPntType _pT
	    ,const pointND<NDIM,int> & _N);

    NDimMem( char * const _pData
	    ,const unsigned long _offset 
	    ,typename base::DatPntType _pT 
	    ,const pointND<NDIM,int> &_N);	
    
    // dereference operator is no longer needed if the 
    // pointer which carry a pointer to an instance of
    // an actual object are introduced
    NDimMem * operator->();
    
    // assignment operator for const NDimMem rhs -> data is copied
    NDimMem & operator=(const NDimMem &);
    // copy constructor for const NDimMem rhs -> const assignment is called
    NDimMem(const NDimMem &);
    // assignment operator for NDimMem rhs -> data is not copied, ptrs
    // point on the data of the rhs
    NDimMem & operator=(NDimMem &);
    // copy constructor for NDimMem rhs -> assignemnt is called
    NDimMem(NDimMem &);  
    
    // copy constructor for Operator triple
    template<typename T1, typename T2,operation_type OP>
      NDimMem(const Operator_Triple<T1,T2,OP>& X);
      
    // assignment operator for operator triple
    template<typename T1, typename T2,operation_type OP>
      const NDimMem& operator=(const Operator_Triple<T1,T2,OP> & X);
    
    inline NDimMem * duplicateObj() const;
    
    // static function for
    static DataType getElem(const NDimMem<NDIM,DataType> &, const int &);
    
  private:
    
    template <typename T1, typename T2,operation_type OP>
      void resolve_op_triple( const NDimMem<NDIM,DataType> ** NDimMemObjPtr
                             ,const Operator_Triple<T1,T2,OP> & X );
			     			     
    template <typename T1, typename T2,operation_type OP>
      void resolve_op_triple( const NDimMem<NDIM,DataType> ** NDimMemObjPtr
	      	             ,const Operator_Triple<T1,T2,OP> & X
			     ,Int2Type<true> );
    template <typename T1, typename T2,operation_type OP>
      void resolve_op_triple( const NDimMem<NDIM,DataType> ** NDimMemObjPtr
			     ,const Operator_Triple<T1,T2,OP> & X
			     ,Int2Type<false> );	 
};

template <short NDIM, typename DataType>
  struct DataTypeDescr<NDimMem<NDIM,DataType> > 
{
  typedef NDimMem<NDIM,DataType> value_type;
  typedef NDimObjPtr<NDIM, NDimMem<NDIM,DataType> > pointer_type;
  typedef NDimMem<NDIM,DataType> & reference_type;
  typedef NDimMem<NDIM,DataType>   reference_return_type;
  static const bool is_standard_type = false;
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//                                                         //
//                 NDimMem Implementation                  //
//                                                         //
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

// default constructor
template <short NDIM, typename DataType>
  NDimMem<NDIM,DataType>::NDimMem()
: base()
{
  
}

// standard constructor
template <short NDIM, typename DataType>
  NDimMem<NDIM,DataType>::NDimMem( typename base::DatPntType _pT
				  ,const pointND<NDIM,int> & _N)
    : base(_pT,_N)
{
}

template <short NDIM, typename DataType>
  NDimMem<NDIM,DataType>::NDimMem( char * const _pData
				  ,const unsigned long _offset 
				  ,typename base::DatPntType _pT 
				  ,const pointND<NDIM,int> &_N)
    : base(_pData,_offset,_pT,_N)
{
}

// reference operator
// template <short NDIM, typename DataType>
//   NDimMemPtr<NDIM,NDimMem<NDIM,DataType> > 
//     NDimMem<NDIM,DataType>::operator&()
// {
//   return NDimMemPtr<NDIM,NDimMem<NDIM,DataType> >( base::pData
// 						  ,base::offset
//                                                   ,base::pT
// 					          ,base::Nlat
// 					          ,base::Ntot);
// }
// 
// // const reference operator
// template <short NDIM, typename DataType>
//   const NDimMemPtr<NDIM,NDimMem<NDIM,DataType> > 
//     NDimMem<NDIM,DataType>::operator&() const
// {
//   return NDimMemPtr<NDIM,NDimMem<NDIM,DataType> >( base::pData
// 						  ,base::offset
//                                                   ,base::pT
// 					          ,base::Nlat
// 					          ,base::Ntot);
// }

// dereference operator
template <short NDIM, typename DataType>
  NDimMem<NDIM,DataType> *
    NDimMem<NDIM,DataType>::operator->()
{
  return this;
}

// assignment operator for const rhs
template <short NDIM, typename DataType>
  NDimMem<NDIM,DataType> &
    NDimMem<NDIM,DataType>::operator=(const NDimMem & _rhs)
{
  
  if( this != _rhs.getConstRawPtr() )
  {
    base::operator=(_rhs);
  }
  
  return *this;
}

// copy constructor for const rhs
template <short NDIM, typename DataType>
  NDimMem<NDIM,DataType>::NDimMem(const NDimMem & _rhs)
  : base(_rhs)
{
  
}

// assignment operator for rhs
template <short NDIM, typename DataType>
  NDimMem<NDIM,DataType> &
    NDimMem<NDIM,DataType>::operator=(NDimMem & _rhs)
{
  if( this != _rhs.getRawPtr() )
  {
    base::operator=(_rhs);
  }
  
  return *this;
  
}

// copy constructor for rhs
template <short NDIM, typename DataType>
  NDimMem<NDIM,DataType>::NDimMem(NDimMem & _rhs)
  : base(_rhs)
{
  
}

template <short NDIM, typename DataType> 
  template <typename T1, typename T2, operation_type OP>
    const NDimMem<NDIM,DataType>& 
      NDimMem<NDIM,DataType>::operator=(const Operator_Triple<T1,T2,OP> & X)
{
  #ifdef DEBUG
    std::cout<<"NDimMem<NDIM,DataType>::operator=( const Operator_Triple<T1,T2,OP> & X)"<<std::endl;
    std::cout<<"Operator_Triple<T1,T2,OP> type ="<<typeid(Operator_Triple<T1,T2,OP>).name()<<std::endl;
    //std::cout<<"Operator_Triple<T1,T2,OP> ="<<X<<std::endl;
    std::cout<<std::endl;
  #endif  
  
  const NDimMem<NDIM,DataType> * VecPtr[X.getNVecElem()];
  
  X.getVecElemPtrs(VecPtr);
  
#ifndef NDEBUG
  // ... check that all NDimMemBase objects are equal ...
  for(int i = 1; i < X.getNVecElem(); ++i)
  {
    if( !( (*VecPtr[i]) == (*VecPtr[i-1]) ) )
    {
      std::stringstream ss;
      ss<<"size of operands do not match:"<<std::endl;
      ss<<"  ptrs["<<i<<"]->Nlat = "<<VecPtr[i]->getN()<<std::endl;
      ss<<"  ptrs["<<i-1<<"]->Nlat = "<<VecPtr[i-1]->getN()<<std::endl;
      std::runtime_error(ss.str());
    } 
  }
#endif
  // ... check that the NDimObj is different from all of the other objects
  //     if yes, assign memory, if not, simply keep the old memory
  bool self_assignment = false;
  
  for( int i = 0; i < X.getNVecElem(); ++i)
  {
    if ( this == VecPtr[i] ) self_assignment = true;
  }
  
  if( ! self_assignment )
  {
	  *this = *VecPtr[0];
  }
    
  resolve_op_triple(VecPtr,X);
    
//     for(int j = 0; j < this->Ntot ; ++j)
//     {
//       #ifdef DEBUG
// // 	for(int i = 0; i < X.getNVecElem();++i)
// // 	{
// // 	  std::cout<<"Obj_"<<i<<"["<<j<<"]"<<VecPtr[i]->operator[](j)<<std::endl;
// // 	}
//       #endif
//       (*this)[j] = X.template resolve< DataType
// 				      ,const int &
// 				      ,NDimMem<NDIM,DataType>
// 				      ,& NDimMem<NDIM,DataType>::getElem>(j);
//     }
  
  return *this;
}

template <short NDIM, typename DataType>
  template <typename T1, typename T2,operation_type OP> 
    NDimMem<NDIM,DataType>::NDimMem(const Operator_Triple<T1,T2,OP> & X)
    : base()
{
  #ifdef DEBUG
    std::cout<<"NDimMem<NDIM,DataType>::NDimMem(const Operator_Triple<T1,T2,OP> & X)"<<std::endl;
  #endif
  operator=(X);
}

template <short NDIM, typename DataType>
  inline
    NDimMem<NDIM,DataType> * NDimMem<NDIM,DataType>::duplicateObj() const
{
  
  #ifdef DEBUG
    std::cout<<"NDimMem.duplicateObj() with"<<std::endl;
    std::cout<<"pData ="<<(void *)base::getpData()<<std::endl;
    std::cout<<"offset="<<base::offset<<std::endl;
    std::cout<<"pT    ="<<(void *)base::pT<<std::endl;
    std::cout<<"Nlat  ="<<base::Nlat<<std::endl;
    std::cout<<"is called ..."<<std::endl;
  #endif
  NDimMem<NDIM,DataType> * pObj 
    = new NDimMem<NDIM,DataType>( base::getpData()
                                   ,base::offset
				 ,base::pT 
				 ,base::Nlat );
  #ifdef DEBUG				 
    std::cout<<"NDimMem.duplicateObj() is finished"<<std::endl;
  #endif  
  return pObj;
  
}



template <short NDIM, typename DataType>
  inline
    DataType NDimMem<NDIM,DataType>::getElem(const NDimMem<NDIM,DataType> & Obj, const int & j)
{
  return Obj[j];
}

template <short NDIM, typename DataType> 
  template <typename T1, typename T2, operation_type OP>
    void
      NDimMem<NDIM,DataType>::resolve_op_triple( const NDimMem<NDIM,DataType> ** NDimMemObjPtr
                                                ,const Operator_Triple<T1,T2,OP> & X )
{
   resolve_op_triple( NDimMemObjPtr
                     ,X 
 		     ,Int2Type<DataTypeDescr<DataType>::is_standard_type>() );
}


/// resolve_op_triple function for elementary data types
template <short NDIM, typename DataType> 
  template <typename T1, typename T2, operation_type OP>
    void
      NDimMem<NDIM,DataType>::resolve_op_triple( const NDimMem<NDIM,DataType> ** NDimMemObjPtr
                                                ,const Operator_Triple<T1,T2,OP> & X 
						,Int2Type<true>)
{
  // the Data type is of standard type
  
  const DataType * DataPtr[X.getNVecElem()];
  
  const int Ntot_      = base::Ntot;
  
  // loop over all elements 
  for( int j (0) ; j < Ntot_ ; ++j )
  {
    
    // loop over all NDimMem objects
    for( int i (0) ; i < X.getNVecElem() ; ++i )
    {
      DataPtr[i] = NDimMemObjPtr[i]->base::pT + j;
    }
    
    (*this)[j] = OpTripleFuncs<Operator_Triple<T1,T2,OP> >::resolve(DataPtr,X);

  }
  
}

/// resolve_op_triple function for non-standard data types
template <short NDIM, typename DataType> 
  template <typename T1, typename T2, operation_type OP>
    void
      NDimMem<NDIM,DataType>::resolve_op_triple( const NDimMem<NDIM,DataType> ** NDimMemObjPtr
                                                ,const Operator_Triple<T1,T2,OP> & X 
                                                ,Int2Type<false>)
{
  // the Data type is not of standard type
  
  #ifdef DEBUG
    std::cout<<std::endl;
    std::cout<<"NDimMem::resolve_op_triple (non-standard data type)"<<std::endl;
    std::cout<<"  There are "<<X.getNVecElem()<<" vector elements involved ..."<<std::endl;
    std::cout<<std::endl;
  #endif
  
  const DataType * DataPtr    [X.getNVecElem() + 1];
  char           *  DataMemAddr [X.getNVecElem() + 1];
  size_t             DataSize   [X.getNVecElem() + 1];
 
  const int Ntot_      = base::Ntot;
 
  // initialize the DataPtr array
  for( int i (0) ; i < X.getNVecElem() ; ++i )
  {
    //std::cout<<"NDimMemObjPtr["<<i<<"] is given by"<<std::endl;
    //std::cout<<*NDimMemObjPtr[i]<<std::endl;
    //std::cout<<"initializing DatPtr["<<i<<"]"<<std::endl;
    DataPtr[i]     = reinterpret_cast<DataType *>
        (NDimMemObjPtr[i]->base::pT->duplicateObj());
    //std::cout<<"initializing DatPtr["<<i<<"] is finished"<<std::endl;
    DataMemAddr[i] = DataPtr[i]->getpData();
    DataSize[i]    = DataPtr[i]->getObjMemSize();
  }
  
  //std::cout<<"initializing DatPtr[] for this"<<std::endl;
  DataPtr[X.getNVecElem()]     = reinterpret_cast<DataType *>
        (this->base::pT->duplicateObj());
  DataMemAddr[X.getNVecElem()] = DataPtr[X.getNVecElem()]->getpData();
  DataSize[X.getNVecElem()]    = DataPtr[X.getNVecElem()]->getObjMemSize();
  
  #ifdef DEBUG
    std::cout<<std::endl;
    std::cout<<"Array initalization finished"<<std::endl;
    std::cout<<std::endl;
  #endif
  
  // loop over all elements 
  for( int j (0) ; j < Ntot_ ; ++j )
  {
    
    #ifdef DEBUG
      std::cout<<"Looping over the elements of NDimMem: loopindex = "<<j<<std::endl;
      std::cout<<std::endl;
    #endif
    
    // loop over all NDimMem objects
    DataType ** DataPtrNonConst = const_cast<DataType **>(DataPtr);
    for( int i (0) ; i < X.getNVecElem() + 1 ; ++i )
    {
      DataPtrNonConst[i]->SetDataMemAddr(DataMemAddr[i] + j * DataSize[i]);
    }
    #ifdef DEBUG
      std::cout<<std::endl;
      std::cout<<"New memory address for the elements are set"<<std::endl;
      std::cout<<std::endl;
    #endif 
    
    //(*this)[j] =
    *(DataPtrNonConst[X.getNVecElem()]) = 
                  OpTripleFuncs<Operator_Triple<T1,T2,OP> >::resolve(DataPtr,X);
    
  }
  
  #ifdef DEBUG
    std::cout<<std::endl;
    std::cout<<"Loop over the elements of NDimMem: finished"<<std::endl;
    std::cout<<"Clean up of initialized array ..."<<std::endl;
    std::cout<<std::endl;
  #endif
  
  // clear the DataPtr array
  for( int i (0) ; i < X.getNVecElem() + 1 ; ++i )
  {
    delete DataPtr[i];
  }
  
  #ifdef DEBUG
    std::cout<<std::endl;
    std::cout<<"Clean up of initialized array finished"<<std::endl;
    std::cout<<"Leaving NDimMem::resolve_op_triple (non-standard data type) ..."<<std::endl;
    std::cout<<std::endl;
  #endif
}

#endif
