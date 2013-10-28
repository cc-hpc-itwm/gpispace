#ifndef GSPC_KVS_KVS_HPP
#define GSPC_KVS_KVS_HPP

#include <gspc/kvs/api.hpp>
#include <boost/shared_ptr.hpp>

namespace gspc
{
  namespace kvs
  {
    boost::shared_ptr<api_t> create (std::string const&);
  }
}

#endif
