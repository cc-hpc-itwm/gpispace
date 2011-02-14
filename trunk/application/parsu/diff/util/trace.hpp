
#ifndef _TRACE_HPP
#define _TRACE_HPP 1

#include <NDimData.h>
#include <pointND.h>

#include <traceheader.hpp>

namespace seislib
{
  class Trace : public NDimData<1, float, TraceHeader>
  {
  public:
    typedef NDimData<1, float, TraceHeader> super;

    // Constructor
    Trace (char * buf, const int & nt)
      : super ( buf
              , reinterpret_cast<float *>(buf)
              , pointND<1, int> (nt)
              )
    {}

    // Copy constructor for an operator triple argument
    template<typename T1, typename T2, operation_type OP>
    Trace (const Operator_Triple<T1,T2,OP> & X)
     : super (X)
    {}
    
    // Assignment operator for an operator triple argument
    template<typename T1, typename T2, operation_type OP>
    Trace & operator = (const Operator_Triple<T1,T2,OP> & X)
    {
     super::operator = (X);
     return *this;
    }

    // Zero copy cast operator from super class to Trace
    Trace (const super & x)
      : super (x.getpData(),x.getpT(),x.getN())
    {}

//     NDimDataPtr<1, super> operator & ()
//     {
//       return super::operator & ();
//     }

  };
}

// Data type description structure for the trace object

template<>
struct DataTypeDescr<seislib::Trace>
{
  typedef seislib::Trace value_type;
  //typedef NDimData<1,float,seislib::TraceHeader> value_type;
  typedef NDimObjPtr<1, NDimData<1,float,seislib::TraceHeader> > pointer_type;
  typedef seislib::Trace & reference_type;
  //typedef NDimData<1,float,seislib::TraceHeader> & reference_type;
  typedef seislib::Trace reference_return_type;
  //typedef NDimData<1,float,seislib::TraceHeader> reference_return_type;
  static const bool is_standard_type = false;
};

#endif
