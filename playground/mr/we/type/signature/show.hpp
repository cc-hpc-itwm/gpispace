// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_SIGNATURE_SHOW_HPP
#define PNET_SRC_WE_TYPE_SIGNATURE_SHOW_HPP

#include <we/type/signature.hpp>

#include <iosfwd>

std::ostream& operator<< ( std::ostream&
                         , const pnet::type::signature::signature_type&
                         );

#endif
