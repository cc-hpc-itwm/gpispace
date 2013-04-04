// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_NUM_HPP
#define _FHG_UTIL_NUM_HPP

#include <fhg/util/parse/position.hpp>

#include <boost/variant.hpp>

#include <iosfwd>

namespace fhg
{
  namespace util
  {
    unsigned long read_ulong (parse::position&);
    unsigned int read_uint (parse::position&);
    long read_long (parse::position&);
    int read_int (parse::position&);
    double read_double (parse::position&);
    float read_float (parse::position&);

    typedef boost::variant< int
                          , long
                          , unsigned int
                          , unsigned long
                          , float
                          , double
                          > num_type;

    num_type read_num (parse::position&);
  }
}

std::ostream& operator<< (std::ostream&, const fhg::util::num_type&);

#endif
