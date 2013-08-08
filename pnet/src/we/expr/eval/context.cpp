// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/eval/context.hpp>

#include <we/type/value/find.hpp>
#include <we/type/value/put.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/mk_structured.hpp>
#include <we/type/value/missing_binding.hpp>

#include <we2/type/compat.hpp>
#include <we2/type/value/poke.hpp>
#include <we2/type/value/peek.hpp>

#include <boost/foreach.hpp>

#include <iostream>

#include <fhg/util/split.hpp>

namespace expr
{
  namespace eval
  {
    namespace
    {
      void do_bind ( boost::unordered_map<std::string, value::type>& container
                   , const std::list<std::string>& key_vec
                   , const value::type& value
                   )
      {
        if (key_vec.empty())
        {
          throw std::runtime_error ("context::bind []");
        }

        std::list<std::string>::const_iterator key_pos (key_vec.begin());
        const std::string& key (*key_pos); ++key_pos;

        value::put ( key_pos
                   , key_vec.end()
                   , value::mk_structured_or_keep (container[key])
                   , value
                   );
      }
      void do_bind2 ( boost::unordered_map< std::string
                                          , pnet::type::value::value_type
                                          >& container
                    , const std::list<std::string>& key_vec
                    , const pnet::type::value::value_type& value
                    )
      {
        if (key_vec.empty())
        {
          throw std::runtime_error ("context::bind []");
        }

        std::list<std::string>::const_iterator key_pos (key_vec.begin());
        const std::string& key (*key_pos); ++key_pos;

        pnet::type::value::poke ( key_pos
                                , key_vec.end()
                                , container[key]
                                , value
                                );
      }
      void do_bind2 ( boost::unordered_map< std::string
                                          , pnet::type::value::value_type
                                          >& container
                    , const std::string& path
                    , const pnet::type::value::value_type& value
                    )
      {
        do_bind2 ( container
                 , fhg::util::split< std::string
                                   , std::list<std::string>
                                   > (path, '.')
                 , value
                 );
      }
    }

    void context::bind ( const std::string& path
                       , const pnet::type::value::value_type& value
                       )
    {
      BIND_OLD (path, pnet::type::compat::COMPAT (value));
      do_bind2 (_container2, path, value);
    }
    void context::bind_ref ( const std::string& path
                           , const pnet::type::value::value_type& value
                           )
    {
      BIND_OLD (path, pnet::type::compat::COMPAT (value));
      do_bind2 (_container2, path, value);
    }

    void context::BIND_OLD (const std::string& path, const value::type& value)
    {
      do_bind ( _container
              , fhg::util::split< std::string
                                , std::list<std::string>
                                > (path, '.')
              , value
              );
      do_bind2 (_container2, path, pnet::type::compat::COMPAT (value));
    }

    void context::bind_and_discard_ref ( const std::list<std::string>& key_vec
                                       , const pnet::type::value::value_type& value
                                       )
    {
      do_bind (_container, key_vec, pnet::type::compat::COMPAT (value));
      do_bind2 (_container2, key_vec, value);
    }

    const value::type& context::value (const std::string& key) const
    {
      const container_type::const_iterator pos (_container.find (key));

      if (pos != _container.end())
      {
        return pos->second;
      }

      throw value::exception::missing_binding (key);
    }

    const pnet::type::value::value_type&
      context::value2 (const std::string& key) const
    {
      const container2_type::const_iterator pos (_container2.find (key));

      if (pos != _container2.end())
      {
        return pos->second;
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

      const container_type::const_iterator pos (_container.find (key));

      if (pos != _container.end())
      {
        return value::find (key_pos, key_vec.end(), pos->second);
      }

      throw value::exception::missing_binding (key);
    }

    const pnet::type::value::value_type&
      context::value2 (const std::list<std::string>& key_vec) const
    {
      if (key_vec.empty())
      {
        throw std::runtime_error ("context::value []");
      }

      std::list<std::string>::const_iterator key_pos (key_vec.begin());
      const std::string& key (*key_pos); ++key_pos;

      {
        const container2_type::const_iterator pos (_container2.find (key));

        if (pos != _container2.end())
        {
          boost::optional<const pnet::type::value::value_type&>
            v (pnet::type::value::peek (key_pos, key_vec.end(), pos->second));

          if (v)
          {
            return *v;
          }
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

      BOOST_FOREACH (const kv_type& kv, cntx._container)
      {
        s << kv.first << " := " << kv.second << std::endl;
      }

      return s;
    }
  }
}
