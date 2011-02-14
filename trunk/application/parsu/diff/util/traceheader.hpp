
#ifndef _TRACE_HEADER_HPP
#define _TRACE_HEADER_HPP 1

#include <iostream>
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
    
    friend std::ostream& operator<<( std::ostream & _os,const TraceHeader & _HeaderData)
    {
      const SegYHeader * pSegYData = _HeaderData.getMetaInfo();
      _os<<std::endl;
      _os<<"*************************************"<<std::endl;
      _os<<"***         Trace Header:         ***"<<std::endl;
      _os<<std::setiosflags(std::ios::left);
      _os<<"*** sx = "<<std::setw(10)<<pSegYData->sx<<"               ***"<<std::endl;
      _os<<"*** sy = "<<std::setw(10)<<pSegYData->sy<<"               ***"<<std::endl;
      _os<<"*** gx = "<<std::setw(10)<<pSegYData->gx<<"               ***"<<std::endl;
      _os<<"*** gy = "<<std::setw(10)<<pSegYData->gy<<"               ***"<<std::endl;
      _os<<"***                               ***"<<std::endl;
      _os<<"*************************************"<<std::endl;
      
      return _os;
    }
    
  };
}

#endif
