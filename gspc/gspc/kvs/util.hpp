#ifndef GSPC_KVS_UTIL_HPP
#define GSPC_KVS_UTIL_HPP

#include <gspc/kvs/kvs.hpp>

namespace gspc
{
  namespace kvs
  {
    int query ( std::string const &url
              , api_t::key_type const &target
              , api_t::value_type const &rqst
              , api_t::key_type const &queue
              , api_t::value_type & result
              , int timeout
              );

    int query ( std::string const &url
              , api_t::key_type const &target
              , api_t::value_type const &rqst
              , api_t::key_type const &queue
              , api_t::value_type & result
              );

    int query ( api_t & kvs
              , api_t::key_type const &target
              , api_t::value_type const &rqst
              , api_t::key_type const &queue
              , api_t::value_type & result
              , int timeout
              );

    int query ( api_t & kvs
              , api_t::key_type const &target
              , api_t::value_type const &rqst
              , api_t::key_type const &queue
              , api_t::value_type & result
              );
  }
}

#endif
