#include "kvs.hpp"

#include <gspc/kvs/impl/kvs_impl.hpp>

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
        throw std::runtime_error ("invalid url for gspc::kvs::create()");
      }
    }
  }
}
