#ifndef GSPC_KVS_NET_FRONTEND_HPP
#define GSPC_KVS_NET_FRONTEND_HPP

#include <boost/thread/mutex.hpp>

#include <gspc/kvs/api.hpp>
#include <gspc/net/client_fwd.hpp>

#include <boost/asio/io_service.hpp>

namespace gspc
{
  namespace kvs
  {
    class kvs_net_frontend_t : public api_t
    {
    public:
      kvs_net_frontend_t (std::string const &url, boost::asio::io_service&);
    private:
      int request ( std::string const &rpc
                  , std::string const &rqst
                  , std::string &rply
                  , size_t timeout = (size_t)0
                  ) const;

      int do_put (std::list<std::pair<key_type, value_type> > const &);

      int do_get (key_type const &key, value_type &val) const;
      int do_get_regex ( std::string const &regex
                       , std::list<std::pair<key_type, value_type> > &
                       ) const;

      int do_del (key_type const &key);
      int do_del_regex (std::string const &regex);

      int do_set_ttl (key_type const &key, int ttl);
      int do_set_ttl_regex (std::string const &regex, int ttl);

      int do_push (key_type const &key, value_type const &val);
      int do_try_pop (key_type const &, value_type &val);

      int do_counter_reset (key_type const &key, int val);
      int do_counter_change (key_type const &key, int &val, int delta);

      int do_wait (key_type const &key, int mask, int timeout_in_ms) const;

      gspc::net::client_ptr_t m_client;
      mutable boost::mutex m_mutex;
    };
  }
}

#endif