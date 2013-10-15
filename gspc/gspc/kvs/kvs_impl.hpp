#ifndef GSPC_KVS_KVS_HPP
#define GSPC_KVS_KVS_HPP

#include <gspc/kvs/api.hpp>

#include <boost/signal.hpp>
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
      int get (key_type const &key, value_type &val) const;
      int del (key_type const &key);

      int push (key_type const &key, value_type const &val);
      int try_pop (key_type const &, value_type &val);

      int counter_reset     (key_type const &key, int  val);

      int counter_increment (key_type const &key, int &val, int delta);
      int counter_increment (key_type const &key, int &val);

      int counter_decrement (key_type const &key, int &val, int delta);
      int counter_decrement (key_type const &key, int &val);
    private:
      typedef boost::shared_mutex mutex_type;

      int lookup (key_type const &, value_type &val);
      int lookup (key_type const &, value_type &val) const;

      mutable mutex_type          m_mutex;

      std::map<key_type, value_type> m_values;
      // std::map<key_type, wait_object*> m_waiting_for_changes;
    };
  }
}

#endif
