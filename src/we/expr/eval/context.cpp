// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/expr/eval/context.hpp>

#include <we/exception.hpp>

#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/show.hpp>

#include <util-generic/join.hpp>

#include <iostream>

namespace expr
{
  namespace eval
  {
    void context::bind_ref ( std::string const& key
                           , pnet::type::value::value_type const& value
                           )
    {
      _ref_container[key] = &value;
    }

    void context::bind_and_discard_ref ( std::list<std::string> const& key_vec
                                       , pnet::type::value::value_type const& value
                                       )
    {
      if (key_vec.empty())
      {
        throw std::runtime_error ("context::bind_and_discard_ref []");
      }

      auto key_pos (key_vec.begin());
      std::string const& key (*key_pos); ++key_pos;

      auto const pos (_ref_container.find (key));

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

    pnet::type::value::value_type const&
      context::value (std::list<std::string> const& key_vec) const
    {
      if (key_vec.empty())
      {
        throw std::runtime_error ("context::value []");
      }

      auto key_pos (key_vec.begin());
      std::string const& key (*key_pos); ++key_pos;

      {
        auto const pos (_ref_container.find (key));

        if (pos != _ref_container.end())
        {
          ::boost::optional<pnet::type::value::value_type const&>
            v (pnet::type::value::peek (key_pos, key_vec.end(), *pos->second));

          if (v)
          {
            return *v;
          }
        }
      }

      {
        auto const pos (_container.find (key));

        if (pos != _container.end())
        {
          ::boost::optional<pnet::type::value::value_type const&>
            v (pnet::type::value::peek (key_pos, key_vec.end(), pos->second));

          if (v)
          {
            return *v;
          }
        }
      }

      throw pnet::exception::missing_binding
        (fhg::util::join (key_vec, ".").string());
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
