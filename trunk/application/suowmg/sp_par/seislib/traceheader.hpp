
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

    TraceHeader operator + (const TraceHeader & rhs) const
    {
      // construct the resulting Trace Header
      TraceHeader Res = *this;

      // introduce some helper pointers
      const SegYHeader * pSegYData_LHS =     getMetaInfo();
      const SegYHeader * pSegYData_RHS = rhs.getMetaInfo();
            SegYHeader * pSegYData_RES = Res.getMetaInfo();

      // number of time samples
      assert( pSegYData_LHS->ns == pSegYData_RHS->ns );
      // sampling rate
      assert( pSegYData_LHS->dt == pSegYData_RHS->dt );
      // T0
      assert( pSegYData_LHS->delrt == pSegYData_RHS->delrt );

      // source coordinates
      pSegYData_RES->sx = pSegYData_LHS->sx + pSegYData_RHS->sx;
      pSegYData_RES->sy = pSegYData_LHS->sy + pSegYData_RHS->sy;

      // receiver coordinates
      pSegYData_RES->gx = pSegYData_LHS->gx + pSegYData_RHS->gx;
      pSegYData_RES->gy = pSegYData_LHS->gy + pSegYData_RHS->gy;

      pSegYData_RES->selev = pSegYData_LHS->selev + pSegYData_RHS->selev;
      pSegYData_RES->gelev = pSegYData_LHS->gelev + pSegYData_RHS->gelev;

      assert( pSegYData_LHS->cdp == pSegYData_RHS->cdp );

      return Res;
    }

    TraceHeader operator - (const TraceHeader & rhs) const
    {
      // construct the resulting Trace Header
      TraceHeader Res = *this;

      // introduce some helper pointers
      const SegYHeader * pSegYData_LHS =     getMetaInfo();
      const SegYHeader * pSegYData_RHS = rhs.getMetaInfo();
      SegYHeader * pSegYData_RES = Res.getMetaInfo();

      // number of time samples
      assert( pSegYData_LHS->ns == pSegYData_RHS->ns );
      // sampling rate
      assert( pSegYData_LHS->dt == pSegYData_RHS->dt );
      // T0
      assert( pSegYData_LHS->delrt == pSegYData_RHS->delrt );

      // source coordinates
      pSegYData_RES->sx = pSegYData_LHS->sx - pSegYData_RHS->sx;
      pSegYData_RES->sy = pSegYData_LHS->sy - pSegYData_RHS->sy;

      // receiver coordinates
      pSegYData_RES->gx = pSegYData_LHS->gx - pSegYData_RHS->gx;
      pSegYData_RES->gy = pSegYData_LHS->gy - pSegYData_RHS->gy;

      pSegYData_RES->selev = pSegYData_LHS->selev - pSegYData_RHS->selev;
      pSegYData_RES->gelev = pSegYData_LHS->gelev - pSegYData_RHS->gelev;

      assert( pSegYData_LHS->cdp == pSegYData_RHS->cdp );

      return Res;
    }

    TraceHeader operator * (const TraceHeader &) const { return *this; }

    TraceHeader operator * (const super::data_type & _scalar) const
    {
      // construct the resulting Trace Header
      TraceHeader Res = *this;

      // introduce some helper pointers
      const SegYHeader * pSegYData_LHS =     getMetaInfo();
      SegYHeader * pSegYData_RES = Res.getMetaInfo();

      // source coordinates
      pSegYData_RES->sx = pSegYData_LHS->sx * _scalar;
      pSegYData_RES->sy = pSegYData_LHS->sy * _scalar;

      // receiver coordinates
      pSegYData_RES->gx = pSegYData_LHS->gx * _scalar;
      pSegYData_RES->gy = pSegYData_LHS->gy * _scalar;

      pSegYData_RES->selev = pSegYData_LHS->selev * _scalar;
      pSegYData_RES->gelev = pSegYData_LHS->gelev * _scalar;

      return Res;
    }

    friend std::ostream& operator<<( std::ostream & _os,const TraceHeader & _HeaderData)
    {
      const SegYHeader * pSegYData = _HeaderData.getMetaInfo();
      _os<<std::endl;
      _os<<"*************************************"<<std::endl;
      _os<<"***         Trace Header:         ***"<<std::endl;
      _os<<"***                               ***"<<std::endl;
      _os<<std::setiosflags(std::ios::left);
      _os<<"*** ns    = "<<std::setw(10)<<pSegYData->ns<<"            ***"<<std::endl;
      _os<<"*** dt    = "<<std::setw(10)<<pSegYData->dt<<"            ***"<<std::endl;
      _os<<"*** delrt = "<<std::setw(10)<<pSegYData->delrt<<"            ***"<<std::endl;
      _os<<"***                               ***"<<std::endl;
      _os<<"*** sx    = "<<std::setw(10)<<pSegYData->sx<<"            ***"<<std::endl;
      _os<<"*** sy    = "<<std::setw(10)<<pSegYData->sy<<"            ***"<<std::endl;
      _os<<"*** gx    = "<<std::setw(10)<<pSegYData->gx<<"            ***"<<std::endl;
      _os<<"*** gy    = "<<std::setw(10)<<pSegYData->gy<<"            ***"<<std::endl;
      _os<<"***                               ***"<<std::endl;
      _os<<"*** selev = "<<std::setw(10)<<pSegYData->selev<<"            ***"<<std::endl;
      _os<<"*** gelev = "<<std::setw(10)<<pSegYData->gelev<<"            ***"<<std::endl;
      _os<<"***                               ***"<<std::endl;
      _os<<"*************************************"<<std::endl;

      return _os;
    }

    int & getsx()
    {
      SegYHeader * pSegYData = getMetaInfo();

      return pSegYData->sx;
    }

    int & getsy()
    {
      SegYHeader * pSegYData = getMetaInfo();

      return pSegYData->sy;
    }

    int & getgx()
    {
      SegYHeader * pSegYData = getMetaInfo();

      return pSegYData->gx;
    }

    int & getgy()
    {
      SegYHeader * pSegYData = getMetaInfo();

      return pSegYData->gy;
    }

  };
}

#endif
