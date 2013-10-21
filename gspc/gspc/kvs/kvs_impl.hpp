#ifndef GSPC_KVS_KVS_HPP
#define GSPC_KVS_KVS_HPP

#include <map>

#include <gspc/kvs/api.hpp>

#include <boost/signal.hpp>
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
      virtual ~kvs_t ();

      boost::signal<void (key_type const &)> onChange;

      int put (key_type const &key, value_type const &val);
      int put (std::list<std::pair<key_type, value_type> > const &);

      int get (key_type const &key, value_type &val) const;
      int get_regex ( std::string const &regex
                    , std::list<std::pair<key_type, value_type> > &
                    ) const;

      int del (key_type const &key);
      int del_regex (std::string const &regex);

      int set_ttl (key_type const &key, int ttl);
      int set_ttl_regex (std::string const &regex, int ttl);

      int push (key_type const &key, value_type const &val);
      int pop (key_type const &, value_type &val);
      int try_pop (key_type const &, value_type &val);

      int counter_reset     (key_type const &key, int  val);

      int counter_increment (key_type const &key, int &val, int delta);
      int counter_increment (key_type const &key, int &val);

      int counter_decrement (key_type const &key, int &val, int delta);
      int counter_decrement (key_type const &key, int &val);
    private:
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
        int try_pop (value_type &);
        int push (value_type const &);
      private:
        int is_value_available () const;

        mutable boost::recursive_mutex mutex;
        boost::condition_variable_any value_changed;
        value_type value;
        int        expiry;
      };

      typedef boost::shared_ptr<entry_t> entry_ptr_t;
      typedef std::map<key_type, entry_ptr_t> value_map_t;

      void purge_expired_keys () const;

      mutable mutex_type  m_mutex;
      value_map_t         m_values;

      mutable mutex_type  m_expired_entries_mutex;
      mutable std::list<key_type> m_expired_entries;
    };
  }
}

#endif
