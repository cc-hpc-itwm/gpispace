
#ifndef _TRACE_BUNCH_HPP
#define _TRACE_BUNCH_HPP 1

// user defined includes
#include <pointND.h>
#include <NDimMem.h>
#include <trace.hpp>

namespace seislib
{
  class TraceBunch : public NDimMem<1, Trace>
  {
  public:
    
    typedef NDimMem<1, Trace> super;

    // Constructor
    TraceBunch ( char * buf
               , const int & N
               , const int & nt
               )
      : super ( &(Trace (buf, nt))
              , pointND<1, int> (N)
              )
    {}

    // Copy constructor for an operator triple argument
    template<typename T1, typename T2, operation_type OP>
    TraceBunch (const Operator_Triple<T1,T2,OP> & X)
      : super (X)
    {}
    
    // Assignment operator for an operator triple argument
    template<typename T1, typename T2, operation_type OP>
    TraceBunch & operator=(const Operator_Triple<T1,T2,OP> & X)
    {
      super::operator=(X);
      return *this;
    }

    // Zero copy cast operator from super class to Trace
    TraceBunch (NDimMem<1, Trace> & x)
      : super (x.getpT(),x.getN())
    {}
  };
}

template<>
struct DataTypeDescr<seislib::TraceBunch>
{
  typedef seislib::TraceBunch value_type;
  typedef NDimObjPtr<1, seislib::Trace > pointer_type;
  typedef seislib::TraceBunch & reference_type;
  typedef seislib::TraceBunch reference_return_type;
  static const bool is_standard_type = false;
};
#endif
