#pragma once

#include <logging/tcp_endpoint.hpp>

#include <util-generic/serialization/by_member.hpp>

#include <boost/serialization/utility.hpp>

FHG_UTIL_SERIALIZATION_BY_MEMBER ( fhg::logging::tcp_endpoint
                                 , &fhg::logging::tcp_endpoint::host
                                 , &fhg::logging::tcp_endpoint::port
                                 );
