#ifndef GENERIC_OPERATOR
#define GENERIC_OPERATOR

/// std includes:

/// user defined includes:
#include "base.h"
#include "operator_triple.h"


/// generic operator structure:
/// The generic operator structure implements 
/// an arithemetic operation, i.e. addition
/// subraction, multiplication or scalar multiplication
/// for two generic operands of type T1 and T2. If one
/// of the operands is a base derived object, the return
/// type is an operator triple structure. Otherwise, the
/// return type is of the type of the left operand


/// Determineing return type for generic_operator

/// If the type of the left or of the right operand is a derived 
/// type of the Base class, then, the return type is an Operator_Triple
/// type. However, if both of the operands are not derived from the 
/// base class, then the return type corresponds to the type of the 
/// left operand


/// structure to obtain the derived type
template <typename T, bool isBaseDerived>
  struct getDerived 
  { };

template <typename T>
  struct getDerived<T,true>
  { typedef typename T::DerivedType type; };

template <typename T>
  struct getDerived<T,false>
  { typedef T type; };

template <typename T>
  struct detOperatorTriple 
  { typedef typename getDerived<T,IsBaseDerived<T>::value >::type type; };


// structure to determine the return type of an arithmetic operation.
// if one of the operands is a type derived from Base, then the 
// return type is an operator triple object. The operand type of the 
// base class derived object is the type represented by the base class
// derived type.
// otherwise, the return type is the type of the left operand

template <typename T1, typename T2, operation_type OP, bool isBaseDerived>
  struct determine_return_type 
  { };

// specialize to is_fundamental = false, i.e. return type
// is an Operator_Triple 
template <typename T1,typename T2,operation_type OP>
struct determine_return_type<T1,T2,OP,true>
{
  //typedef Operator_Triple<T1,T2,OP> ret_type;
  typedef Operator_Triple< typename detOperatorTriple<T1>::type
  ,typename detOperatorTriple<T2>::type
  ,OP> ret_type;
};

// specialize to is_fundamental = true, i.e. return type
// is the fundamental type
template <typename T1, typename T2, operation_type OP>
struct determine_return_type<T1,T2,OP,false>
{
  typedef T1 ret_type;
};

// structure which holds the return type of the 
// generic_operator function
template <class T1, class T2, operation_type OP>
struct eval_ret
{
  
  typedef typename 
  determine_return_type< T1
  ,T2
  ,OP
  , IsBaseDerived<T1>::value ||
  IsBaseDerived<T2>::value
  >::ret_type type;
};

/// generic_operator definition
/// T1: left operator type
/// T2: right operator type
/// OP: operation type

template <typename T1, typename T2,operation_type OP>
struct generic_operator {};

/// specialization of the generic_operator defintion to operation type ADD
template <typename T1, typename T2>
struct generic_operator<T1,T2,ADD>
{
  inline static
  const typename eval_ret<T1,T2,ADD>::type 
  eval(const T1 & operand1, const T2 & operand2)
  {
    #ifdef DEBUG1
    //       typedef const typename eval_ret<T1,T2,ADD>::type RetType;
    //       std::cout<<"generic_operator<ADD>:"<<std::endl;
    //       std::cout<<"operand1 type:"<<typeid(T1).name()
    // 	       <<" value:"<<operand1<<std::endl;
    //       std::cout<<"operand2 type:"<<typeid(T2).name()
    // 	       <<" value:"<<operand2<<std::endl;
    //       std::cout<<"return type  :"<<typeid(RetType).name()<<std::endl<<std::endl;
    #endif
    return operand1+operand2;
  }
};

/// specialization of the generic_operator defintion to operation type SUB
template <typename T1, typename T2>
struct generic_operator<T1,T2,SUB>
{
  inline static
  const typename eval_ret<T1,T2,SUB>::type  
  eval(const T1 & operand1, const T2 & operand2)
  {
    #ifdef DEBUG1
    // 	typedef const typename eval_ret<T1,T2,SUB>::type RetType;
    // 	std::cout<<"generic_operator<SUB>:"<<std::endl;
    // 	std::cout<<"operand1 type:"<<typeid(T1).name()
    // 		 <<" value:"<<operand1<<std::endl;
    // 	std::cout<<"operand2 type:"<<typeid(T2).name()
    // 		 <<" value:"<<operand2<<std::endl;
    // 	std::cout<<"return type  :"<<typeid(RetType).name()<<std::endl<<std::endl;
    #endif
    return operand1-operand2;
  }
};

/// specialization of the generic_operator defintion to operation type MUL
template <typename T1, typename T2>
struct generic_operator<T1,T2,MUL>
{
  inline static
  const typename eval_ret<T1,T2,MUL>::type 
  eval(const T1 & operand1, const T2 & operand2)
  {
    #ifdef DEBUG1
    // 	typedef const typename eval_ret<T1,T2,MUL>::type RetType;
    // 	std::cout<<"generic_operator<MUL>:"<<std::endl;
    // 	std::cout<<"operand1 type:"<<typeid(T1).name()
    // 	         <<" value:"<<operand1<<std::endl;
    // 	std::cout<<"operand2 type:"<<typeid(T2).name()
    // 	         <<" value:"<<operand2<<std::endl;
    // 	std::cout<<"return type  :"<<typeid(RetType).name()<<std::endl<<std::endl;
    #endif
    return operand1*operand2;
  }
};

/// specialization of the generic_operator defintion to operation type SCALMUL
template <typename T1, typename T2>
struct generic_operator<T1,T2,SCALMUL>
{
  inline static
  const typename eval_ret<T1,T2,SCALMUL>::type 
  eval(const T1 & operand1, const T2 & operand2)
  {
    #ifdef DEBUG1
    // 	typedef const typename eval_ret<T1,T2,SCALMUL>::type RetType;
    // 	std::cout<<"generic_operator<SCALMUL>:"<<std::endl;
    // 	std::cout<<"operand1 type:"<<typeid(T1).name()
    // 	         <<" value:"<<operand1<<std::endl;
    // 	std::cout<<"operand2 type:"<<typeid(T2).name()
    // 	         <<" value:"<<operand2<<std::endl;
    // 	std::cout<<"return type  :"<<typeid(RetType).name()<<std::endl<<std::endl;
    #endif
    return operand1*operand2;
  }
};


#endif