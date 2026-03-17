#pragma once

#include <gspc/util/hash/combined_hash.hpp>

#include <boost/asio/ip/tcp.hpp>

FHG_UTIL_MAKE_COMBINED_STD_HASH ( ::boost::asio::ip::tcp::endpoint
                                , endpoint
                                , endpoint.address().to_string()
                                , endpoint.port()
                                )
