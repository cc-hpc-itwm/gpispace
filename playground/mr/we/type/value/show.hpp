// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_SHOW_HPP
#define PNET_SRC_WE_TYPE_VALUE_SHOW_HPP

#include <we/type/value.hpp>

#include <iosfwd>

std::ostream& operator<< (std::ostream&, const pnet::type::value::value_type&);

#endif
