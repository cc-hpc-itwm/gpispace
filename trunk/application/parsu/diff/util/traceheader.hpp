
#ifndef _TRACE_HEADER_HPP
#define _TRACE_HEADER_HPP 1

#include <SegYHeader.h>

#include <MetaInfo.h>

namespace seislib
{
  class TraceHeader : public MetaInfoBase<SegYHeader, float>
  {
  private:
    typedef MetaInfoBase<SegYHeader, float> super;

  public:
    TraceHeader (): super (){}
    TraceHeader (const char * buf) : super (buf) {}
    TraceHeader (const TraceHeader & x) : super(x) {}

    TraceHeader operator + (const TraceHeader &) const { return *this; }
    TraceHeader operator - (const TraceHeader &) const { return *this; }
    TraceHeader operator * (const TraceHeader &) const { return *this; }
    TraceHeader operator * (const typename super::data_type &) const { return *this; }
  };
}

#endif
