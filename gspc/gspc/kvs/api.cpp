#include "api.hpp"
#include <errno.h>

namespace gspc
{
  namespace kvs
  {
    bool api_t::is_key_valid (key_type const &k) const
    {
      if (k.empty () || k.find (' ') != key_type::npos)
        return false;
      return true;
    }

    int api_t::put (key_type const &key, value_type const &val)
    {
      if (not is_key_valid (key))
        return -EKEYREJECTED;

      std::list<std::pair<key_type, value_type> > kv_pairs;
      kv_pairs.push_back (std::list<std::pair<key_type, value_type> >::value_type (key,val));
      return this->do_put (kv_pairs);
    }

    int api_t::put (std::list<std::pair<key_type, value_type> > const &lst)
    {
      return this->do_put (lst);
    }

    int api_t::get (key_type const &key, value_type &val) const
    {
      if (not is_key_valid (key))
        return -EKEYREJECTED;

      return this->do_get (key, val);
    }

    int api_t::get_regex ( std::string const &regex
                         , std::list<std::pair<key_type, value_type> > &vals
                         ) const
    {
      return this->do_get_regex (regex, vals);
    }

    int api_t::del (key_type const &key)
    {
      if (not is_key_valid (key))
        return -EKEYREJECTED;

      return this->do_del (key);
    }

    int api_t::del_regex (std::string const &regex)
    {
      return this->do_del_regex (regex);
    }

    int api_t::set_ttl (key_type const &key, int ttl)
    {
      if (not is_key_valid (key))
        return -EKEYREJECTED;

      return this->do_set_ttl (key, ttl);
    }

    int api_t::set_ttl_regex (std::string const &regex, int ttl)
    {
      return this->do_set_ttl_regex (regex, ttl);
    }

    int api_t::push (key_type const &key, value_type const &val)
    {
      if (not is_key_valid (key))
        return -EKEYREJECTED;

      return this->do_push (key, val);
    }
    int api_t::pop (key_type const &key, value_type &val)
    {
      if (not is_key_valid (key))
        return -EKEYREJECTED;

      return this->do_pop (key, val);
    }
    int api_t::try_pop (key_type const &key, value_type &val)
    {
      if (not is_key_valid (key))
        return -EKEYREJECTED;

      return this->do_try_pop (key, val);
    }

    int api_t::counter_reset     (key_type const &key, int  val)
    {
      if (not is_key_valid (key))
        return -EKEYREJECTED;

      return this->do_counter_reset (key, val);
    }
    int api_t::counter_increment (key_type const &key, int &val, int delta)
    {
      if (not is_key_valid (key))
        return -EKEYREJECTED;

      return this->do_counter_change (key, val, delta < 0 ? -delta : delta);
    }
    int api_t::counter_increment (key_type const &key, int &val)
    {
      if (not is_key_valid (key))
        return -EKEYREJECTED;

      return this->do_counter_change (key, val, 1);
    }

    int api_t::counter_decrement (key_type const &key, int &val, int delta)
    {
      if (not is_key_valid (key))
        return -EKEYREJECTED;

      return this->do_counter_change (key, val, delta > 0 ? -delta : delta);
    }
    int api_t::counter_decrement (key_type const &key, int &val)
    {
      if (not is_key_valid (key))
        return -EKEYREJECTED;

      return this->do_counter_change (key, val, -1);
    }
  }
}
