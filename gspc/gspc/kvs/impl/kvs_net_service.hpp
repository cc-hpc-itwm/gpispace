#ifndef GSPC_KVS_SERVICE_HPP
#define GSPC_KVS_SERVICE_HPP

#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <gspc/kvs/api.hpp>
#include <gspc/net/service/handler.hpp>

namespace gspc
{
  namespace kvs
  {
    class service_t
    {
    public:
      static const std::string &NAME ()
      {
        static std::string name ("kvs"); return name;
      }

      service_t ();
      ~service_t ();

      void operator () ( std::string const &dst
                       , gspc::net::frame const &rqst
                       , gspc::net::user_ptr user
                       );

      api_t & api ();
    private:
      struct waiting_t
      {
        gspc::kvs::api_t::key_type key;
        int mask;
        int events;
        gspc::net::frame rqst;
      };

      typedef boost::function< boost::optional<gspc::net::frame> (gspc::net::frame const &)> rpc_t;
      typedef std::map<std::string, rpc_t> rpc_table_t;
      typedef std::list<waiting_t> waiting_list_t;

      void setup_rpc_handler ();

      void on_change (api_t::key_type const &, int events);

      boost::optional<gspc::net::frame>
      rpc_put (gspc::net::frame const &);

      boost::optional<gspc::net::frame>
      rpc_get (gspc::net::frame const &);

      boost::optional<gspc::net::frame>
      rpc_get_regex (gspc::net::frame const &);

      boost::optional<gspc::net::frame>
      rpc_del (gspc::net::frame const &);

      boost::optional<gspc::net::frame>
      rpc_del_regex (gspc::net::frame const &);

      boost::optional<gspc::net::frame>
      rpc_set_ttl (gspc::net::frame const &);

      boost::optional<gspc::net::frame>
      rpc_set_ttl_regex (gspc::net::frame const &);

      boost::optional<gspc::net::frame>
      rpc_push (gspc::net::frame const &);

      boost::optional<gspc::net::frame>
      rpc_wait (gspc::net::frame const &);

      boost::optional<gspc::net::frame>
      rpc_try_pop (gspc::net::frame const &);

      boost::optional<gspc::net::frame>
      rpc_counter_reset (gspc::net::frame const &);

      boost::optional<gspc::net::frame>
      rpc_counter_change (gspc::net::frame const &);

      boost::optional<gspc::net::frame>
      rpc_help (gspc::net::frame const &);

      boost::optional<gspc::net::frame>
      rpc_invalid (gspc::net::frame const &);

      gspc::net::frame
      encode_error_code (gspc::net::frame const &rqst, int rc);

      api_t *m_kvs;
      rpc_table_t m_rpc_table;

      mutable boost::shared_mutex m_waiting_mtx;
      waiting_list_t m_waiting;

      boost::signals2::connection m_on_change_connection;
    };
  }
}

#endif
