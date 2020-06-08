#include <we/expr/eval/context.hpp>

#include <we/exception.hpp>

#include <we/type/value/poke.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/show.hpp>

#include <iostream>

namespace expr
{
  namespace eval
  {
    void context::bind_ref ( const std::string& key
                           , const pnet::type::value::value_type& value
                           )
    {
      _ref_container[key] = &value;
    }

    void context::bind_and_discard_ref ( const std::list<std::string>& key_vec
                                       , const pnet::type::value::value_type& value
                                       )
    {
      if (key_vec.empty())
      {
        throw std::runtime_error ("context::bind_and_discard_ref []");
      }

      std::list<std::string>::const_iterator key_pos (key_vec.begin());
      const std::string& key (*key_pos); ++key_pos;

      const ref_container_type::const_iterator pos (_ref_container.find (key));

      if (pos != _ref_container.end())
      {
        _container[key] = *pos->second;
        _ref_container.erase (pos);
      }

      pnet::type::value::poke ( key_pos
                              , key_vec.end()
                              , _container[key]
                              , value
                              );
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
        const ref_container_type::const_iterator pos (_ref_container.find (key));

        if (pos != _ref_container.end())
        {
          boost::optional<const pnet::type::value::value_type&>
            v (pnet::type::value::peek (key_pos, key_vec.end(), *pos->second));

          if (v)
          {
            return *v;
          }
        }
      }

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

    std::ostream& operator<< (std::ostream& os, context const& c)
    {
      for (auto const& kr : c._ref_container)
      {
        os << kr.first << " ~> " << pnet::type::value::show (*kr.second) << '\n';
      }
      for (auto const& kv : c._container)
      {
        os << kv.first << " -> " << pnet::type::value::show (kv.second) << '\n';
      }

      return os;
    }
  }
}
