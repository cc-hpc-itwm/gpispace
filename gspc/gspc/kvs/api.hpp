#ifndef GSPC_KVS_API_HPP
#define GSPC_KVS_API_HPP

#include <list>
#include <utility>
#include <string>
#include <we/type/value.hpp>

namespace gspc
{
  namespace kvs
  {
    class api_t
    {
    public:
      static const int EXPIRES_NEVER = -1;

      virtual ~api_t () {}

      typedef std::string                   key_type;
      typedef pnet::type::value::value_type value_type;

      int put (key_type const &key, const char *val);
      int put (key_type const &key, value_type const &val);
      int put (std::list<std::pair<key_type, value_type> > const &lst);

      int get (key_type const &key, value_type &val) const;
      int get_regex ( std::string const &
                    , std::list<std::pair<key_type, value_type> > &
                    ) const;

      int del (key_type const &key);
      int del_regex (std::string const &regex);

      int set_ttl (key_type const &key, int ttl);
      int set_ttl_regex (std::string const &regex, int ttl);

      int push (key_type const &key, const char *val);
      int push (key_type const &key, value_type const &val);
      int pop (key_type const &, value_type &val);
      int try_pop (key_type const &, value_type &val);

      int counter_reset  (key_type const &key, int  val);
      int counter_change (key_type const &key, int &val, int delta);
      int counter_increment (key_type const &key, int &val);
      int counter_decrement (key_type const &key, int &val);
    private:
      bool is_key_valid (key_type const &) const;

      virtual int do_put (std::list<std::pair<key_type, value_type> > const &lst) = 0;
      virtual int do_get (key_type const &key, value_type &val) const = 0;
      virtual int do_get_regex ( std::string const &regex
                               , std::list<std::pair<key_type, value_type> > &vals
                               ) const = 0;

      virtual int do_del (key_type const &key) = 0;
      virtual int do_del_regex (std::string const &regex) = 0;

      virtual int do_set_ttl (key_type const &key, int ttl) = 0;
      virtual int do_set_ttl_regex (std::string const &regex, int ttl) = 0;

      virtual int do_push (key_type const &key, value_type const &val) = 0;
      virtual int do_pop (key_type const &, value_type &val) = 0;
      virtual int do_try_pop (key_type const &, value_type &val) = 0;

      virtual int do_counter_reset  (key_type const &key, int  val) = 0;
      virtual int do_counter_change (key_type const &key, int &val, int delta) = 0;
    };
  }
}

#endif
