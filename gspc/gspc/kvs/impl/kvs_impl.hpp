#ifndef GSPC_KVS_KVS_IMPL_HPP
#define GSPC_KVS_KVS_IMPL_HPP

#include <map>

#include <gspc/kvs/api.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/shared_mutex.hpp>

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

      virtual ~kvs_t ();
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
      int do_pop (key_type const &, value_type &val, int timeout);
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

        bool is_expired () const;
        void expires_in (int ttl);

        void set_value (value_type const &);
        value_type & get_value ();
        value_type const & get_value () const;

        int pop (value_type &);
        int pop (value_type &, int timeout);
        int try_pop (value_type &);
        int push (value_type const &);
      private:
        int is_value_available () const;

        mutable boost::recursive_mutex mutex;
        boost::condition_variable_any value_changed;
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

#endif
