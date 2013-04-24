// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_NUM_SHOW_HPP
#define _FHG_UTIL_NUM_SHOW_HPP

#include <fhg/util/num.hpp>

#include <iosfwd>

std::ostream& operator<< (std::ostream&, const fhg::util::num_type&);

#endif
