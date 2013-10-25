#ifndef GSPC_KVS_NET_FRONTEND_HPP
#define GSPC_KVS_NET_FRONTEND_HPP

#include <gspc/kvs/api.hpp>
#include <gspc/net/client_fwd.hpp>

namespace gspc
{
  namespace kvs
  {
    class kvs_net_frontend_t : public api_t
    {
    public:
      explicit
      kvs_net_frontend_t (std::string const &url);

      virtual ~kvs_net_frontend_t ();

    private:
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
      int do_pop (key_type const &, value_type &val);
      int do_try_pop (key_type const &, value_type &val);

      int do_counter_reset (key_type const &key, int val);
      int do_counter_change (key_type const &key, int &val, int delta);

      std::string m_url;
      gspc::net::client_ptr_t m_client;
    };
  }
}

#endif
