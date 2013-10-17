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

      virtual int put (key_type const &key, value_type const &val) = 0;
      virtual int put (std::list<std::pair<key_type, value_type> > const &) = 0;

      virtual int get (key_type const &key, value_type &val) const = 0;
      virtual int get_regex ( std::string const &regex
                            , std::list<std::pair<key_type, value_type> > &
                            ) const = 0;

      virtual int del (key_type const &key) = 0;
      virtual int del_regex (std::string const &regex) = 0;

      virtual int set_ttl (key_type const &key, int ttl) = 0;
      virtual int set_ttl_regex (std::string const &regex, int ttl) = 0;

      virtual int push (key_type const &key, value_type const &val) = 0;
      virtual int try_pop (key_type const &, value_type &val) = 0;

      virtual int counter_reset     (key_type const &key, int  val) = 0;

      virtual int counter_increment (key_type const &key, int &val, int delta) = 0;
      virtual int counter_increment (key_type const &key, int &val) = 0;

      virtual int counter_decrement (key_type const &key, int &val, int delta) = 0;
      virtual int counter_decrement (key_type const &key, int &val) = 0;
    };
  }
}

#endif
