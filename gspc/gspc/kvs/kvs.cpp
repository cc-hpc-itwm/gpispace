#include "kvs.hpp"

#include <gspc/kvs/impl/kvs_impl.hpp>
#include <gspc/kvs/impl/kvs_net_frontend.hpp>

namespace gspc
{
  namespace kvs
  {
    api_t *create (std::string const &url)
    {
      if (url.find ("inproc://") == 0)
      {
        return new gspc::kvs::kvs_t (url);
      }
      else
      {
        return new gspc::kvs::kvs_net_frontend_t (url);
      }
    }
  }
}
