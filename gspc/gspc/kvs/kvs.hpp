#ifndef GSPC_KVS_KVS_HPP
#define GSPC_KVS_KVS_HPP

#include <gspc/kvs/api.hpp>

namespace gspc
{
  namespace kvs
  {
    api_t *create (std::string const&);

    int     initialize (std::string const &url);
    int     shutdown ();
    api_t & get ();
  }
}

#endif
