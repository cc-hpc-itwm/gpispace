#include "util.hpp"
#include <boost/shared_ptr.hpp>

namespace gspc
{
  namespace kvs
  {
    int query ( std::string const &url
              , api_t::key_type const &target
              , api_t::value_type const &rqst
              , api_t::key_type const &queue
              , api_t::value_type & result
              )
    {
      return query (url, target, rqst, queue, result, -1);
    }

    int query ( std::string const &url
              , api_t::key_type const &target
              , api_t::value_type const &rqst
              , api_t::key_type const &queue
              , api_t::value_type & result
              , int timeout
              )
    {
      boost::shared_ptr<api_t> kvs (gspc::kvs::create (url));
      return query (*kvs, target, rqst, queue, result, timeout);
    }

    int query ( api_t & kvs
              , api_t::key_type const &target
              , api_t::value_type const &rqst
              , api_t::key_type const &queue
              , api_t::value_type & result
              )
    {
      return query (kvs, target, rqst, queue, result, -1);
    }

    int query ( api_t & kvs
              , api_t::key_type const &target
              , api_t::value_type const &rqst
              , api_t::key_type const &queue
              , api_t::value_type & result
              , int timeout
              )
    {
      int rc;

      rc = kvs.push (target, rqst);
      if (rc != 0)
        return rc;

      rc = kvs.pop (queue, result, timeout);
      if (rc != 0)
        return rc;

      return 0;
    }
  }
}
