#include "kvs.hpp"

#include <gspc/kvs/impl/kvs_impl.hpp>
#include <gspc/kvs/impl/kvs_net_frontend.hpp>

namespace gspc
{
  namespace kvs
  {
    boost::shared_ptr<api_t> create (std::string const &url)
    {
      if (url.find ("inproc://") == 0)
      {
        return
          boost::shared_ptr<api_t>(new gspc::kvs::kvs_t (url));
      }
      else
      {
        return
          boost::shared_ptr<api_t>(new gspc::kvs::kvs_net_frontend_t (url));
      }
    }
  }
}
