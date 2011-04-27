#ifndef BASE
#define BASE

/// std includes:

/// user defined includes:
#include "convertibility_check.h"

/// Definition of the base class from which all of the other
/// classes which are supposed to take advantage of the zero
/// copy and minimal loop employment philosophy have to be 
/// derived

/// The class implements type definitions for a ScalarType 
/// and a VectorType which are intended to ensure type
/// safety for the arithmetic oeprations.
///

struct CheckBaseDerived
{
  /// Check base derived is intended to be the base class
  /// for the following Base structure definition.
  /// It is introduced as a helper struct to have a zero 
  /// parameter "template" which can be used to check whether 
  /// a generic class is derived from the base class 
  /// or not.  
};

template <typename T1>
  struct IsBaseDerived
{
  /// The structure evaluates whether the type T1 is
  /// derived from the base class or not. In order
  /// to do so, it checks whether the T1 is derived
  /// from the CheckBaseDerived structure.
  
  /// Assumption: The base class is derived from
  ///             CheckBaseDerived
  enum {value = INHERITS_STRICT(CheckBaseDerived,T1) };
};

template <typename derived, typename ScalarType_In, typename VectorType_In>
  struct Base : public CheckBaseDerived
{
  
  /// definition of the base class.
  
  /// type definition of the scalar type 
  typedef ScalarType_In                          ScalarType;
  /// type definition of the vector type
  typedef VectorType_In                          VectorType;
  /// type definition of the 
  typedef derived 				 DerivedType;
  
  /// function to cast
  const derived & get_ref() const
  {
    return static_cast<const derived &>(*this);
  }
  
  derived & get_nonconst_ref() const
  {
    return const_cast<derived &>(get_ref());
  }

};


template <typename derived, typename ScalarType_In>
    class Base_Obj : public Base< derived, ScalarType_In, derived >
{
   /// Simple wrapper class to the Base class 
   /// for N-dimensional objects. This class is 
   /// intended to reduce the total number of 
   /// template parameters of the Base class by
   /// one
  public:
    
   /// get raw pointer to derived class 
   const derived * getConstRawPtr() const
   { return static_cast<const derived *>(this); }
   
   /// get const raw pointer to derived class
   derived * getRawPtr()
   { return static_cast<derived *>(this); }
   
};


#endif

#ifndef BASE_OP_DEFS
#define BASE_OP_DEFS

#include "operator_triple.h"

/// Arithmetic operator definition for base class objects.
/// these definitions do also apply for derived objects.
/// Provided operations include:
///
///   operator+      (addition of two base-derived objects)
///   operator+=     (addition asignment of two base-derived objects)
///   operator-      (subtraction of two base-derived objects)
///   operator-=     (subtraction assignment of two base-derived objects)
///   operator*      (multiplication of two base-derived objects)
///   operator*=     (multiplication assignment of two base-derived objects)
///   operator*      (multiplication of a base-derived object with a scalar.
///                   This operator comes in two flavors, namely
///                   scalar left - and scalar right multiplication)
///   operator*=     (multiplication assignment of a base-derived object with a scalar.
///                   This operator implements scalar right multiplication 
///                   assignment)

/// the return value for all of these arithmetic operations
/// is an Operator_Triple object which holds both a reference
/// to the left and the right operand.

/// Addition operator definition
template <typename T1, typename T2>
    inline
        const Operator_Triple<T1,T2,ADD>
        operator+( const Base< T1
                              ,typename T1::ScalarType
                              ,typename T1::VectorType > & operand1
                  ,const Base< T2
                              ,typename T1::ScalarType
                              ,typename T1::VectorType > & operand2)
{
#ifdef DEBUG
  std::cout<<"operator+(const Base<"<<typeid(T1).name()<<"> & operand1, const Base<"<<typeid(T2).name()<<"> & operand2)"<<std::endl;
#endif
  return Operator_Triple<T1,T2,ADD>( operand1.get_ref(), operand2.get_ref() );
}

/// Addition assignment operator definition
template <typename T1, typename T2>
    inline
      T1 &
      operator+=( const Base< T1
	                     ,typename T1::ScalarType
	                     ,typename T1::VectorType > & operand1
	         ,const Base< T2
                             ,typename T1::ScalarType
                             ,typename T1::VectorType > & operand2)
{
  #ifdef DEBUG
  std::cout<<"operator+=(const Base<"<<typeid(T1).name()<<"> & operand1, const Base<"<<typeid(T2).name()<<"> & operand2)"<<std::endl;
  #endif
  operand1.get_nonconst_ref() 
      = Operator_Triple<T1,T2,ADD>( operand1.get_ref(), operand2.get_ref() );
  return operand1.get_nonconst_ref();
}

/// Subtraction operator definition
template <typename T1, typename T2>
    inline
        const Operator_Triple<T1,T2,SUB>
        operator-( const Base< T1
                              ,typename T1::ScalarType
                              ,typename T1::VectorType > & operand1
                  ,const Base< T2
                              ,typename T1::ScalarType
                              ,typename T1::VectorType > & operand2)
{
#ifdef DEBUG
  std::cout<<"operator-(const Base<"<<typeid(T1).name()<<"> & operand1, const Base<"<<typeid(T2).name()<<"> & operand2)"<<std::endl;
#endif
  return Operator_Triple<T1,T2,SUB>( operand1.get_ref(), operand2.get_ref() );
}

/// Subtraction assignment operator definition
template <typename T1, typename T2>
    inline
	T1 &
	operator-=(       Base< T1
			       ,typename T1::ScalarType
                               ,typename T1::VectorType > & operand1
                   ,const Base< T2
                               ,typename T1::ScalarType
                               ,typename T1::VectorType > & operand2)
{
  #ifdef DEBUG
  std::cout<<"operator-=(const Base<"<<typeid(T1).name()<<"> & operand1, const Base<"<<typeid(T2).name()<<"> & operand2)"<<std::endl;
  #endif
  operand1.get_nonconst_ref() = 
      Operator_Triple<T1,T2,SUB>( operand1.get_ref(), operand2.get_ref() );
  
  return operand1.get_nonconst_ref();
}

/// Multiplication operator definition
template <typename T1, typename T2>
    inline
        const Operator_Triple<T1,T2,MUL>
        operator*( const Base< T1
                              ,typename T1::ScalarType
                              ,typename T1::VectorType > & operand1
                  ,const Base< T2
                              ,typename T1::ScalarType
                              ,typename T1::VectorType > & operand2)
{
#ifdef DEBUG
  std::cout<<"operator*(const Base<"<<typeid(T1).name()<<"> & operand1, const Base<"<<typeid(T2).name()<<"> & operand2)"<<std::endl;
#endif
  return Operator_Triple<T1,T2,MUL>( operand1.get_ref(), operand2.get_ref() );
}

/// Multiplication assignment operator definition
template <typename T1, typename T2>
    inline
      T1 &
      operator*=(       Base< T1
			    ,typename T1::ScalarType
			    ,typename T1::VectorType > & operand1
		 ,const Base< T2
			    ,typename T1::ScalarType
			    ,typename T1::VectorType > & operand2)
{
  #ifdef DEBUG
  std::cout<<"operator*(const Base<"<<typeid(T1).name()<<"> & operand1, const Base<"<<typeid(T2).name()<<"> & operand2)"<<std::endl;
  #endif
  operand1.get_nonconst_ref() = 
      Operator_Triple<T1,T2,MUL>( operand1.get_ref(), operand2.get_ref() );
  
  return operand1.get_nonconst_ref();
}

/// Scalar right multiplication definition
template <typename T1>
    inline
    const Operator_Triple<T1,typename T1::ScalarType,SCALMUL>
    operator*( const Base< T1
                          ,typename T1::ScalarType
                          ,typename T1::VectorType > & operand1
                ,const typename T1::ScalarType & scalar)
{
#ifdef DEBUG
  std::cout<<"operator*(const Base<"
           <<typeid(T1).name()<<"> & operand1, const "
           <<typeid(typename T1::ScalarType).name()<<"> & val)"<<std::endl;
#endif
  
  return Operator_Triple<T1,typename T1::ScalarType,SCALMUL>
      ( operand1.get_ref(), scalar );
}

/// Scalar right multiplication assignment definition
template <typename T1>
    inline
    T1 &
	operator*=( Base< T1
			,typename T1::ScalarType
			,typename T1::VectorType > & operand1
		  ,const typename T1::ScalarType & scalar)
{
  #ifdef DEBUG
  std::cout<<"operator*=(const Base<"
  <<typeid(T1).name()<<"> & operand1, const "
  <<typeid(T1::ScalarType).name()<<"> & val)"<<std::endl;
  #endif
  operand1.get_nonconst_ref() 
      = Operator_Triple<T1,typename T1::ScalarType,SCALMUL>( operand1.get_ref(), scalar ); 
      
  return operand1.get_non_const_ref(); 
}

/// Scalar left multiplication definition
template <typename T1>
    inline
    const Operator_Triple<T1,typename T1::ScalarType,SCALMUL>
    operator*( const typename T1::ScalarType & scalar 
              ,const Base< T1
                          ,typename T1::ScalarType
                          ,typename T1::VectorType > & operand1)
{
#ifdef DEBUG
  std::cout<<"operator*(const "
           <<typeid(typename T1::ScalarType).name()<<"> & val, const Base<"
           <<typeid(T1).name()<<"> & operand1)"<<std::endl;
#endif
   
  return Operator_Triple<T1,typename T1::ScalarType,SCALMUL>
      ( operand1.get_ref(), scalar );
}

// /// Scalar left multiplication assignment definition
// template <typename T1>
//     inline
// 	T1 &
// 	operator*=( const typename T1::ScalarType & scalar 
// 		   ,      Base< T1
// 			       ,typename T1::ScalarType
//                                ,typename T1::VectorType > & operand1)
// {
//   #ifdef DEBUG
//   std::cout<<"operator*=(const "
//   <<typeid(T1::ScalarType).name()<<"> & val, const Base<"
//   <<typeid(T1).name()<<"> & operand1)"<<std::endl;
//   #endif
//   operand1.get_nonconst_ref() 
//       = Operator_Triple<T1,typename T1::ScalarType,SCALMUL>( operand1.get_ref(), scalar );
//       
//   return operand1.get_non_const_ref();
// }

#endif
