
#ifndef _TRACE_BUNCH_HPP
#define _TRACE_BUNCH_HPP 1

#include <NDimMem.h>

#include <trace.hpp>

#include <pointND.h>

namespace seislib
{
  class TraceBunch : public NDimMem<1, Trace>
  {
  public:
    typedef NDimMem<1, Trace> super;

    TraceBunch ( char * buf
               , const int & N
               , const int & nt
               )
      : super ( &(Trace (buf, nt))
              , pointND<1, int> (N)
              )
    {}

    template<typename T1, typename T2, operation_type OP>
    TraceBunch (const Operator_Triple<T1,T2,OP> & X)
      : super (X)
    {}

    TraceBunch (const NDimMem<1, Trace> & x)
      : super (x)
    {}
  };
}

#endif
