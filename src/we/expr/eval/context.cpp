// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <we/expr/eval/context.hpp>

#include <we/exception.hpp>

#include <we/type/value/poke.hpp>
#include <we/type/value/peek.hpp>
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

      std::list<std::string>::const_iterator key_pos (key_vec.begin());
      std::string const& key (*key_pos); ++key_pos;

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

    pnet::type::value::value_type const&
      context::value (std::list<std::string> const& key_vec) const
    {
      if (key_vec.empty())
      {
        throw std::runtime_error ("context::value []");
      }

      std::list<std::string>::const_iterator key_pos (key_vec.begin());
      std::string const& key (*key_pos); ++key_pos;

      {
        const ref_container_type::const_iterator pos (_ref_container.find (key));

        if (pos != _ref_container.end())
        {
          boost::optional<pnet::type::value::value_type const&>
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
          boost::optional<pnet::type::value::value_type const&>
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
