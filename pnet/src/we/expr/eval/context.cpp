// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/eval/context.hpp>

#include <we/exception.hpp>

#include <we/type/value/poke.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/show.hpp>

#include <boost/foreach.hpp>

#include <iostream>

#include <fhg/util/split.hpp>

namespace expr
{
  namespace eval
  {
    namespace
    {
      void do_bind ( boost::unordered_map< std::string
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
      void do_bind ( boost::unordered_map< std::string
                                          , pnet::type::value::value_type
                                          >& container
                    , const std::string& path
                    , const pnet::type::value::value_type& value
                    )
      {
        do_bind ( container
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
      do_bind (_container, path, value);
    }
    void context::bind_ref ( const std::string& path
                           , const pnet::type::value::value_type& value
                           )
    {
      do_bind (_container, path, value);
    }

    void context::bind_and_discard_ref ( const std::list<std::string>& key_vec
                                       , const pnet::type::value::value_type& value
                                       )
    {
      do_bind (_container, key_vec, value);
    }

    const pnet::type::value::value_type&
      context::value (const std::string& key) const
    {
      const container_type::const_iterator pos (_container.find (key));

      if (pos != _container.end())
      {
        return pos->second;
      }

      throw pnet::exception::missing_binding (key);
    }

    const pnet::type::value::value_type&
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
          boost::optional<const pnet::type::value::value_type&>
            v (pnet::type::value::peek (key_pos, key_vec.end(), pos->second));

          if (v)
          {
            return *v;
          }
        }
      }

      throw pnet::exception::missing_binding (key);
    }

    std::ostream& operator<< (std::ostream& s, const context& cntx)
    {
      typedef std::pair<std::string, pnet::type::value::value_type> kv_type;

      BOOST_FOREACH (const kv_type& kv, cntx._container)
      {
        s << kv.first
          << " := " << pnet::type::value::show (kv.second)
          << std::endl;
      }

      return s;
    }
  }
}
