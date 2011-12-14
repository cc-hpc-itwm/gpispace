#ifndef OPERATOR_TRIPLE_DEFINITION
#define OPERATOR_TRIPLE_DEFINITION

/// std includes
#include <typeinfo>

/// user defined includes
#include "base.h"
#include "convertibility_check.h"

/// Definition of the available arithmetic operation types
/// ADD		: add two vector elements
/// SUB		: subtract two vector elements
/// MUL		: multiply two vector elements
/// SCALMUL	: scalar multiplication of vector element with a scalar element
enum operation_type {ADD,SUB,MUL,SCALMUL};

/// Definition of the Operator_Triple class
/// Triplet of types which represents an arithmetic operation
/// T1: left operand type
/// T2: right operand type
/// OP: operation type
template <typename T1, typename T2, operation_type OP>
class Operator_Triple : public Base< Operator_Triple<T1, T2, OP> 
                                    ,typename T1::ScalarType
                                    ,typename T1::VectorType >
{
  
  private:
    
    /// Definition of the operand type 
    template<typename T1_>
      struct operand
      {
        typedef const T1_ & type;
      };
    
    /// Operator triple operand member variable is a 
    /// not a constant reference for Operator_Triple
    /// operands.
    template<typename T1_,typename T2_,operation_type OP_>
      struct operand<Operator_Triple<T1_,T2_,OP_> >
      {
        typedef const Operator_Triple<T1_,T2_,OP_> type;
      };
  
  public:
    
    /// Define first and second operand
    typename operand<T1>::type operand1;
    typename operand<T2>::type operand2;
    
    // standard constructor
    Operator_Triple(const T1 & in_operand1, const T2 & in_operand2);
    // destructor
    ~Operator_Triple();
    
    // copy constructor
    Operator_Triple(const Operator_Triple & rhs);
    // asignment operator
    Operator_Triple & operator=(const Operator_Triple & rhs);

    // Resolve the operator triple object by applying a static 
    // function with zero arguments
    template <typename ResType, typename ObjType, ResType (*F)(const ObjType &) >
      inline
	ResType 
	  resolve() const;
    
    // Resolve the operator triple object
    template <typename ResType, typename ArgType, typename ObjType, ResType (*F)(const ObjType &, ArgType) >
      inline
	ResType 
	  resolve(ArgType Arg) const;
    
    // Resolve the operator triple object by providing 
    // an array of ptrs
//     template <typename DataType>
//       inline 
        
    
    // return the total number of vector elements in the Operator_Triple object. 
    inline int getNVecElem() const; 

    // return the total number of scalar elements in the Operator_Triple object 
    inline int getNScalElem() const;

    
    // fills an array of vector elements with 
    inline void getVecElemPtrs(const typename Operator_Triple<T1,T2,OP>::VectorType ** VecPtrs) const;
    
    // << operator
    template <typename T1_OS, typename T2_OS, operation_type OP_OS>
      friend std::ostream& operator<<( std::ostream &
					    ,const Operator_Triple<T1_OS,T2_OS,OP_OS> &);
};




/// type checking routines for  Operator_Triple

template <bool var> 
  struct is_true 
  { };

template <>
  struct is_true<true>
  { static const bool test = true; };

template <typename T1_IN, typename T2_IN, operation_type OP_IN>
struct assert_types_struct
{
  static const bool check = is_true< Conversion< typename T1_IN::VectorType 
						,typename T2_IN::VectorType>::sameType >::test;
};

template <typename T1_IN, typename T2_IN>
struct assert_types_struct<T1_IN,T2_IN,SCALMUL>
{
  static const bool check = is_true< Conversion< typename T1_IN::ScalarType ,T2_IN >::sameType >::test;
};

template <typename T1_IN, typename T2_IN, operation_type OP_IN>
static void 
assert_types()
{
  bool check_valid_operands = assert_types_struct<T1_IN,T2_IN,OP_IN>::check;
}

#endif

#ifndef OPERATOR_TRIPLE_FUNCS
#define OPERATOR_TRIPLE_FUNCS

#include "generic_operator.h"

template <typename ResType, typename T, bool is_base_derived>
  struct resolve_ret_help
  { };

/// specialization of resolve_ret_help to standard types

template <typename ResType, typename T>
  struct resolve_ret_help<ResType,T,false>
  {
  typedef ResType type;
};


template <typename ResType,typename T>
struct resolve_ret
{
  typedef typename resolve_ret_help<ResType,T,IsBaseDerived<ResType>::value >::type type;
};

/// functions to handle the Operator triple objects
/// generic definition for type T1

template <typename T1>
struct OpTripleFuncs
{
  /// countTotOps() counts the total number of operands in an
  /// operator triple type, i.e. the sum of scalar and vector
  /// operatnds
  inline static int countTotOps()
  {
    return 1; // terminating condition
  }
  
  /// countVecOps counts the number of vector operands
  /// in an operator triple type
  inline static int countVecOps()
  {
    return 1; // terminating condition
  }
  
  /// getOpPtrs returns pointers to the vector objects
  /// in an operator triple object
  template <typename ptr_Type> 
    inline static void
      getOpPtrs(const ptr_Type ** ptrs, const T1 & X)
  {
    ptrs[0] = reinterpret_cast<const ptr_Type * >(X.getConstRawPtr());
  }
  
  /// resolve computes the result represented by an operator
  /// triple object. Here in_ptrs is an array to the objects
  /// which will be used to resolve the operator triple object
  template <typename DataType>
    inline static const DataType &
      resolve(const DataType ** in_ptrs, const T1 & X)
  {
	//std::cout<<"resolve: in_ptrs_addr ="<<(void *)in_ptrs[0]<<std::endl;
    return *(in_ptrs[0]);
  }
    
  template <typename ResType, typename ObjType, ResType (*F)(const ObjType &)>
    inline static ResType
      resolve(const T1 & X)
  {
    const ObjType * pObj = reinterpret_cast<const ObjType * >(&X);
    
    return (*F)(*pObj);
  }
  
  template <typename ResType, typename ArgType, typename ObjType, ResType (*F)(const ObjType &, ArgType)>
    inline static ResType
      resolve(ArgType Arg, const T1 & X)
  {
    const ObjType * pObj = reinterpret_cast<const ObjType * >(&X);
    
    return (*F)(*pObj,Arg);
  }
  
  
};


/// functions to handle the operator triple objects
/// specializations to Operator_Triple template parameters

template <typename T1, typename T2, operation_type OP>
    struct OpTripleFuncs< Operator_Triple<T1,T2,OP> >
{
  
  inline static int countTotOps()
  {
    int  res = 0;
    
    // try to expand the left node (T1) which might be an
    // Operator_Triple type
    res += OpTripleFuncs<T1>::countTotOps();
    
    // try to expand the right node (T2) which might be an
    // Operator_Triple type
    res += OpTripleFuncs<T2>::countTotOps();
    
    return res;
  }
  
  inline static int countVecOps()
  {
    int  res = 0;
    
    // try to expand the left node (T1) which might be an
    // Operator_Triple type
    res += OpTripleFuncs<T1>::countVecOps();
    
    // try to expand the right node (T2) which might be an
    // Operator_Triple type
    res += OpTripleFuncs<T2>::countVecOps();
    
    return res;
  }
  
  template <typename ptr_Type>
      inline static void
	getOpPtrs(const ptr_Type** in_ptrs, const Operator_Triple<T1,T2,OP> & X)
  {
    // traverse the left node
    OpTripleFuncs<T1>::template getOpPtrs<ptr_Type>(&in_ptrs[0], X.operand1);
    
    // determine the number of operators resisting on the left node
    const int idx = OpTripleFuncs<T1>::countVecOps();
    
    // traverse the right node
    OpTripleFuncs<T2>::template getOpPtrs<ptr_Type>(&in_ptrs[idx], X.operand2);

  }
  
  template <typename DataType>
    inline static typename resolve_ret<DataType,Operator_Triple<T1,T2,OP> >::type
      resolve(const DataType ** in_ptrs, const Operator_Triple<T1,T2,OP> & X)
  {
    
    // traverse the left node and compute the intermediate
    // result
    typedef typename resolve_ret<DataType,T1>::type LResType;
    const LResType & lres = OpTripleFuncs<T1>
                              ::template resolve<DataType>(&in_ptrs[0],X.operand1);
    
    // determine the number of operators resisting on the left node
    const int idx = OpTripleFuncs<T1>::countVecOps();
    
    // traverse the right node and compute the intermediate
    // result
    typedef typename resolve_ret<DataType,T2>::type RResType;
    const RResType & rres = OpTripleFuncs<T2>
                              ::template resolve<DataType>(&in_ptrs[idx],X.operand2);
    
    return generic_operator<LResType,RResType,OP>::eval(lres,rres);
    
  }
      
  template <typename ResType, typename ObjType, ResType (*F)(const ObjType &)>
    inline static ResType
      resolve(const Operator_Triple<T1,T2,OP> & X)
  { 
    // traverse the left node and compute the intermediate
    // result
    const ResType  lres = OpTripleFuncs<T1>
			      ::template resolve<ResType, ObjType, F>(X.operand1);
    
    // traverse the right node and compute the intermediate
    // result
    const ResType rres = OpTripleFuncs<T2>
			      ::template resolve<ResType, ObjType, F>(X.operand2);
    
    return generic_operator<ResType,ResType,OP>::eval(lres,rres);
  }
  
  template <typename ResType, typename ArgType, typename ObjType, ResType (*F)(const ObjType &, ArgType)>
    inline static ResType
      resolve(ArgType Arg, const Operator_Triple<T1,T2,OP> & X)
  { 
    // traverse the left node and compute the intermediate
    // result
    // const lrestype = is_base_derived -> lrestype = generate_new_operator_triple::op_trip_type
    //					-> lrestype = ResType
    //
    typedef typename resolve_ret<ResType,T1>::type LResType;
    const ResType  lres = OpTripleFuncs<T1>
			      ::template resolve<ResType,ArgType, ObjType, F>(Arg,X.operand1);
    
    // traverse the right node and compute the intermediate
    // result
    typedef typename resolve_ret<ResType,T2>::type RResType;
    const ResType rres = OpTripleFuncs<T2>
			      ::template resolve<ResType,ArgType, ObjType, F>(Arg,X.operand2);
    
    return generic_operator<ResType,ResType,OP>::eval(lres,rres);
  }
  
};

template <typename T1>
    struct OpTripleFuncs< Operator_Triple<T1,typename T1::ScalarType,SCALMUL> >
{
  inline static int countVecOps()
  {
    int  res = 0;
    
    // try to expand the left node (T1) which might be an
    // Operator_Triple type
    res += OpTripleFuncs<T1>::countVecOps();
    
    // try to expand the right node (T2) which might be an
    // Operator_Triple type
    //res += OpTripleFuncs<T2>::countVecOps();
    
    return res;
  }
  
  template <typename ptr_Type>
    inline static void
      getOpPtrs(const ptr_Type** in_ptrs, const Operator_Triple<T1,typename T1::ScalarType,SCALMUL> & X)
  {
    // traverse the left node
    OpTripleFuncs<T1>::template getOpPtrs<ptr_Type>(&in_ptrs[0], X.operand1);
    
  }
  
  template <typename DataType>
    inline static 
      typename resolve_ret<DataType,Operator_Triple<T1,typename T1::ScalarType,SCALMUL> >::type
        resolve( const DataType ** in_ptrs
	        ,const Operator_Triple<T1,typename T1::ScalarType,SCALMUL> & X)
  {
    
    // traverse the left node and compute the intermediate
    // result
    typedef typename resolve_ret<DataType,T1>::type LResType;
    const LResType & lres = OpTripleFuncs<T1>
                             ::template resolve<DataType>(&in_ptrs[0],X.operand1);
    
    // traverse the right node and compute the intermediate
    // result
    const typename T1::ScalarType & rres = X.operand2;
    
    return generic_operator<LResType,typename T1::ScalarType,SCALMUL>::eval(lres,rres);
    
  }
  
  
  template <typename ResType, typename ObjType, ResType (*F)(const ObjType &)>
    inline static ResType
      resolve(const Operator_Triple<T1,typename T1::ScalarType,SCALMUL> & X)
  { 
    // traverse the left node and compute the intermediate
    // result
    const ResType  lres = OpTripleFuncs<T1>
			    ::template resolve<ResType, ObjType, F>(X.operand1);
    
    // traverse the right node and compute the intermediate
    // result
    const typename T1::ScalarType rres = X.operand2;
    
    return generic_operator<ResType,typename T1::ScalarType,SCALMUL>::eval(lres,rres);
  }
  
  template <typename ResType, typename ArgType, typename ObjType, ResType (*F)(const ObjType &, ArgType)>
    inline static ResType
      resolve( ArgType Arg, const Operator_Triple<T1,typename T1::ScalarType,SCALMUL> & X)
  { 
    // traverse the left node and compute the intermediate
    // result
    
    const ResType  lres = OpTripleFuncs<T1>
			    ::template resolve<ResType,ArgType, ObjType, F>(Arg,X.operand1);
    
    // traverse the right node and compute the intermediate
    // result
    const typename T1::ScalarType rres = X.operand2;
    
    return generic_operator<ResType,typename T1::ScalarType,SCALMUL>::eval(lres,rres);
  }
 
};

/// Cast

/// generate_new_operator_triple struct

template <typename new_type, typename T1>
    struct generate_new_operator_triple
{ 
  typedef typename detOperatorTriple<new_type>::type op_trip_type;
  
  inline static 
      const new_type &
        resolve(const T1 & X)
  {
    const T1 * pObj = reinterpret_cast<const T1 *>(&X);
    const new_type * pNewObj = (const new_type *)(*pObj);
    return *pNewObj;
  }
  
}; 

// Spezialization of the generate_new_operator_triple struct
// to an Operator_Triple type

template <typename new_type, typename T1, typename T2, operation_type OP>
    struct generate_new_operator_triple< new_type, Operator_Triple<T1,T2,OP> >
{
  
  typedef  Operator_Triple< typename generate_new_operator_triple<new_type,T1>
			      ::op_trip_type
			   ,typename generate_new_operator_triple<new_type,T2>
			      ::op_trip_type
			   ,OP> op_trip_type;
  
  inline static
      const typename generate_new_operator_triple<new_type
      ,Operator_Triple<T1,T2,OP> >::op_trip_type 
        resolve(const Operator_Triple<T1,T2,OP> & X)
  {
     
    typedef typename generate_new_operator_triple<new_type,T1>
      ::op_trip_type T_lres;
 
    typedef typename generate_new_operator_triple<new_type,T2>
      ::op_trip_type T_rres;
    
    return generic_operator<T_lres,T_rres,OP>::
        eval( generate_new_operator_triple<new_type,T1>::resolve(X.operand1)
             ,generate_new_operator_triple<new_type,T2>::resolve(X.operand2));
  }
  
};

template <typename new_type, typename T1>
    struct generate_new_operator_triple< new_type, Operator_Triple<T1
        ,typename T1::ScalarType,SCALMUL> >
{
  typedef  Operator_Triple< typename generate_new_operator_triple<new_type,T1>
  ::op_trip_type
  ,typename T1::ScalarType
  ,SCALMUL> op_trip_type;
  
  inline static
      const typename generate_new_operator_triple
        <new_type,Operator_Triple<T1,typename T1::ScalarType,SCALMUL> >
          ::op_trip_type  
      resolve( const Operator_Triple<T1
              ,typename T1::ScalarType,SCALMUL> & X)
      { 
        typedef typename generate_new_operator_triple<new_type,T1>
          ::op_trip_type T_lres;

        return generic_operator<T_lres,typename T1::ScalarType,SCALMUL>::
          eval( generate_new_operator_triple<new_type,T1>::resolve(X.operand1)
	       ,X.operand2);
      }
  
};


/// specialization of resolve_ret_help to base derived type
template <typename ResType, typename T>
  struct resolve_ret_help<ResType,T,true>
{
  typedef typename generate_new_operator_triple<ResType,T>::op_trip_type type;
};

// #endif
// 
// #ifndef OPERATOR_TRIPLE_IMPLEMENTATION
// #define OPERATOR_TRIPLE_IMPLEMENTATION
// 
// #include
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////                                                                                                            ////
////                              Operator_Triple implementation                                                ////
////                                                                                                            ////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// standard constructor
template <typename T1, typename T2, operation_type OP>
  Operator_Triple<T1,T2,OP>::Operator_Triple(const T1 & in_operand1, const T2 & in_operand2)
    : operand1(in_operand1)
     ,operand2(in_operand2)
{
  assert_types<T1,T2,OP>();
  #ifdef DEBUG
//   std::cout<<typeid(Operator_Triple<T1,T2,OP>).name()<<"::Operator_Triple(in_operand1,in_operand2)"<<std::endl;
//   std::cout<<"in_operand1 = "<<in_operand1<<std::endl;
//   std::cout<<"in_operand2 = "<<in_operand2<<std::endl;
//   std::cout<<std::endl;
  #endif 
}

/// copy constructor
template <typename T1, typename T2, operation_type OP>
  Operator_Triple<T1,T2,OP>::Operator_Triple(const Operator_Triple & rhs)
    : operand1(rhs.operand1)
     ,operand2(rhs.operand2)
{
  #ifdef DEBUG
//   std::cout<<typeid(Operator_Triple<T1,T2,OP>).name()<<"::Operator_Triple(const Operator_Triple)"<<std::endl;
//   std::cout<<"Operator_Triple = "<<rhs<<std::endl;
//   std::cout<<std::endl;
  #endif 
}

/// destructor
template <typename T1, typename T2, operation_type OP>
  Operator_Triple<T1,T2,OP>::~Operator_Triple()
{
  #ifdef DEBUG
//   std::cout<<typeid(Operator_Triple<T1,T2,OP>).name()<<"::~Operator_Triple()"<<std::endl;
//   std::cout<<"Operator_Triple = "<<*this<<std::endl;
//   std::cout<<std::endl;
  #endif  
}

/// asignment operator
template <typename T1, typename T2, operation_type OP>
  Operator_Triple<T1,T2,OP> & 
    Operator_Triple<T1,T2,OP>::operator=(const Operator_Triple & rhs)
{
  #ifdef DEBUG
//   std::cout<<typeid(Operator_Triple<T1,T2,OP>).name()<<"::operator=(const Operator_Triple &)"<<std::endl;
//   std::cout<<"Operator_Triple = "<<std::endl;
//   std::cout<<std::endl;
  #endif 
  operand1 = rhs.operand1;
  operand2 = rhs.operand2;
}

/// Resolve the operator triple object with a static 
/// single parameter function who is 
template <typename T1, typename T2, operation_type OP>
  template <typename ResType, typename ObjType, ResType (*F)(const ObjType &) >
    inline
      ResType
	Operator_Triple<T1,T2,OP>::resolve() const
{
  typedef typename Operator_Triple<T1,T2,OP>::ScalarType _ScalarType;
  typedef typename Operator_Triple<T1,T2,OP>::VectorType _VectorType;
  
  const ResType res = OpTripleFuncs< Operator_Triple<T1,T2,OP> >
			    ::template resolve< ResType
					       ,_VectorType
					       ,F>(*this);
  
  return res;
}

/// Resolve the operator triple object with a two parameter function
template <typename T1, typename T2, operation_type OP>
  template <typename ResType, typename ArgType, typename ObjType, ResType (*F)(const ObjType &, ArgType) >
    inline
      ResType
	  Operator_Triple<T1,T2,OP>::resolve(ArgType Arg) const
{
  typedef typename Operator_Triple<T1,T2,OP>::ScalarType _ScalarType;
  typedef typename Operator_Triple<T1,T2,OP>::VectorType _VectorType;
  
  const _ScalarType res = OpTripleFuncs< Operator_Triple<T1,T2,OP> >
			      ::template resolve< ResType 
						 ,ArgType
						 ,_VectorType
						 ,F>(Arg, *this);
  
  return res;
}

/// return the total number of vector elements in the Operator_Triple object.
template <typename T1, typename T2, operation_type OP>
  inline
    int
      Operator_Triple<T1,T2,OP>::getNVecElem() const 
{
  return OpTripleFuncs<Operator_Triple<T1,T2,OP> >::countVecOps();
}

/// return the total number of scalar elements in the Operator_Triple object 
template <typename T1, typename T2, operation_type OP>
  inline
    int
      Operator_Triple<T1,T2,OP>:: getNScalElem() const
{
  return ( OpTripleFuncs<Operator_Triple<T1,T2,OP> >::countTotOps()
	  -OpTripleFuncs<Operator_Triple<T1,T2,OP> >::countVecOps() );
}

/// fills an array of vector elements with 
template <typename T1, typename T2, operation_type OP>
  inline
    void
      Operator_Triple<T1,T2,OP>::getVecElemPtrs(const typename Operator_Triple<T1,T2,OP>::VectorType ** VecPtrs) const
{
  
  // compute the total number of vector elements in the Operator_Triple object
  const int Noperands = getNVecElem();
  
  // fill the array with the actual vector element pointers
  OpTripleFuncs<Operator_Triple<T1,T2,OP> >::getOpPtrs(VecPtrs, *this);
  
}

template <typename T1, typename T2, operation_type OP>
  std::ostream& operator<<( std::ostream & os,const Operator_Triple<T1,T2,OP> & op)
{
  os<<"< "<<typeid(T1).name()<<" , "<<typeid(T2).name()<<" >"<<std::endl;
  //os<<"< "<<op.operand1<<" , "<<op.operand2;
  
  return os;
}

#endif
