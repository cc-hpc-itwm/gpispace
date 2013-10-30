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
      virtual ~service_t ();

      explicit
      service_t (std::string const &url);

      void operator () ( std::string const &dst
                       , gspc::net::frame const &rqst
                       , gspc::net::user_ptr user
                       );
    private:
      struct waiting_to_pop_t
      {
        gspc::kvs::api_t::key_type key;
        gspc::net::frame rqst;
      };

      typedef boost::function< boost::optional<gspc::net::frame> (gspc::net::frame const &)> rpc_t;
      typedef std::map<std::string, rpc_t> rpc_table_t;
      typedef std::list<waiting_to_pop_t> waiting_to_pop_list_t;

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
      rpc_pop (gspc::net::frame const &);

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

      boost::shared_ptr<api_t> m_kvs;
      rpc_table_t m_rpc_table;

      mutable boost::shared_mutex m_waiting_to_pop_mtx;
      waiting_to_pop_list_t m_waiting_to_pop;

      boost::signals2::connection m_on_change_connection;
    };
  }
}

#endif
