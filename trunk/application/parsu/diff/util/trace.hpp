
#ifndef _TRACE_HPP
#define _TRACE_HPP 1

#include <NDimData.h>
#include <pointND.h>

#include <traceheader.hpp>

namespace seislib
{
  class Trace : public NDimData<1, float, TraceHeader, Trace>
  {
  public:
    typedef NDimData<1, float, TraceHeader, Trace> super;

    Trace (char * buf, const int & nt)
      : super ( buf
              , reinterpret_cast<float *>(buf)
              , pointND<1, int> (nt)
              )
    {}

    template<typename T1, typename T2, operation_type OP>
    Trace (const Operator_Triple<T1,T2,OP> & X)
      : super (X)
    {}

    Trace (const NDimData<1, float, TraceHeader, Trace> & x)
      : super (x)
    {}

    NDimDataPtr<1, super> operator & ()
    {
      return super::operator & ();
    }

    template<typename T1, typename T2, operation_type OP>
    Trace & operator = (const Operator_Triple<T1,T2,OP> & X)
    {
      super::operator = (X);
    }
  };
}

template<>
struct DataTypeDescr<seislib::Trace>
{
  typedef seislib::Trace value_type;
  typedef NDimDataPtr<1, NDimData<1,float,seislib::TraceHeader,seislib::Trace> > pointer_type;
  typedef seislib::Trace & reference_type;
  typedef seislib::Trace reference_return_type;

  static const bool is_standard_type = false;
};

#endif
