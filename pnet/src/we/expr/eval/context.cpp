// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/eval/context.hpp>

#include <we/type/value/find.hpp>
#include <we/type/value/put.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/mk_structured.hpp>
#include <we/type/value/missing_binding.hpp>

#include <boost/foreach.hpp>

#include <iostream>

namespace expr
{
  namespace eval
  {
    void context::bind (const std::string& key, const value::type& value)
    {
      _container[key] = value;
    }

    void context::bind ( const std::string& key
                       , const std::list<std::string>& path
                       , const value::type& value
                       )
    {
      value::put ( path
                 , value::mk_structured_or_keep (_container[key])
                 , value
                 );
    }
    void context::bind ( const std::string& key
                       , const std::string& path
                       , const value::type& value
                       )
    {
      value::put ( path
                 , value::mk_structured_or_keep (_container[key])
                 , value
                 );
    }

    void context::bind_and_discard_ref ( const std::list<std::string>& key_vec
                                       , const value::type& value
                                       )
    {
      if (key_vec.empty())
      {
        throw std::runtime_error ("context::bind []");
      }

      std::list<std::string>::const_iterator key_pos (key_vec.begin());
      const std::string& key (*key_pos); ++key_pos;

      value::type& store (value::mk_structured_or_keep (_container[key]));

      const ref_container_type::const_iterator ref_pos
        (_ref_container.find (key));

      if (ref_pos != _ref_container.end())
      {
        store = *ref_pos->second;
        _ref_container.erase (ref_pos);
      }

      value::put (key_pos, key_vec.end(), store, value);
    }

    void context::bind_ref (const std::string& key, const value::type& value)
    {
      _ref_container.insert (std::make_pair (key, &value));
    }

    const value::type& context::value (const std::string& key) const
    {
      {
        const container_type::const_iterator pos (_container.find (key));

        if (pos != _container.end())
        {
          return pos->second;
        }
      }

      {
        ref_container_type::const_iterator pos (_ref_container.find (key));

        if (pos != _ref_container.end())
        {
          return *pos->second;
        }
      }

      throw value::exception::missing_binding (key);
    }

    const value::type&
    context::value (const std::list<std::string>& key_vec) const
    {
      if (key_vec.empty())
      {
        throw std::runtime_error ("context::value []");
      }

      std::list<std::string>::const_iterator key_pos (key_vec.begin());
      const std::string& key (*key_pos); ++key_pos;

      {
        const container_type::const_iterator pos (_container.find (key));

        if (pos != _container.end())
        {
          return value::find (key_pos, key_vec.end(), pos->second);
        }
      }

      {
        ref_container_type::const_iterator pos (_ref_container.find (key));

        if (pos != _ref_container.end())
        {
          return value::find (key_pos, key_vec.end(), *pos->second);
        }
      }

      throw value::exception::missing_binding (key);
    }

    const boost::unordered_map<std::string,value::type>&
    context::values() const
    {
      return _container;
    }

    std::ostream& operator<< (std::ostream& s, const context& cntx)
    {
      typedef std::pair<std::string, value::type> kv_type;
      typedef std::pair<std::string, value::type const*> kp_type;

      BOOST_FOREACH (const kv_type& kv, cntx._container)
      {
        s << kv.first << " := " << kv.second << std::endl;
      }
      BOOST_FOREACH (const kp_type& kp, cntx._ref_container)
      {
        s << kp.first << " -> " << *kp.second << std::endl;
      }

      return s;
    }
  }
}
