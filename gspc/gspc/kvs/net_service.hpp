#ifndef GSPC_KVS_SERVICE_HPP
#define GSPC_KVS_SERVICE_HPP

#include <gspc/kvs/api.hpp>

#include <gspc/net/service/handler.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <map>

namespace gspc
{
  namespace kvs
  {
    class kvs_t : public api_t
    {
    public:
      kvs_t ();

      explicit
      kvs_t (std::string const &url);
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
      int do_try_pop (key_type const &, value_type &val);

      int do_counter_reset (key_type const &key, int val);
      int do_counter_change (key_type const &key, int &val, int delta);

      int do_wait ( key_type const &key
                  , int mask
                  , int timeout_in_ms
                  ) const;

      typedef boost::shared_mutex mutex_type;

      class entry_t
      {
      public:
        explicit
        entry_t (value_type const &value)
          : value (value)
          , expiry (-1)
        {}

        int  is_popable () const;
        bool is_expired () const;
        void expires_in (int ttl);

        void set_value (value_type const &);
        value_type & get_value ();
        value_type const & get_value () const;

        int try_pop (value_type &);
        int push (value_type const &);
      private:
        int is_value_available () const;

        mutable boost::recursive_mutex mutex;
        value_type value;
        int        expiry;
      };

      class waiting_t
      {
      public:
        explicit
        waiting_t (key_type const &key, int mask)
          : m_mutex ()
          , m_cond ()
          , m_key (key)
          , m_mask (mask)
          , m_notified (0)
        {}

        key_type const & key () const { return m_key; }

        int wait ();
        int wait (int timeout);

        void notify (int events);
      private:
        mutable boost::mutex m_mutex;
        boost::condition_variable m_cond;
        const key_type m_key;
        const int m_mask;
        int m_notified;
      };

      typedef boost::shared_ptr<entry_t> entry_ptr_t;
      typedef std::map<key_type, entry_ptr_t> value_map_t;

      void purge_expired_keys () const;
      void notify (key_type const &, int events) const;

      mutable mutex_type  m_mutex;
      value_map_t         m_values;

      mutable mutex_type  m_expired_entries_mutex;
      mutable std::list<key_type> m_expired_entries;

      mutable mutex_type  m_waiting_mutex;
      mutable std::list<waiting_t *> m_waiting;
    };
  }
}

namespace gspc
{
  namespace kvs
  {
    class service_t
    {
    public:
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

      kvs_t m_kvs;
      rpc_table_t m_rpc_table;

      mutable boost::shared_mutex m_waiting_mtx;
      waiting_list_t m_waiting;

      boost::signals2::connection m_on_change_connection;
    };
  }
}

#endif
