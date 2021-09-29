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

#include <we/type/value/poke.hpp>
#include <we/type/value/path/split.hpp>

#include <functional>
#include <iterator>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        struct first_is : public std::unary_function<std::string const&, bool>
        {
          first_is (std::string const& what)
            : _what (what)
          {}

          template<typename T>
          bool operator() (T const& x)
          {
            return x.first == _what;
          }

        private:
          const std::string _what;
        };

        class visitor_poke : public boost::static_visitor<value_type&>
        {
        public:
          visitor_poke ( std::list<std::string>::const_iterator const& key
                       , std::list<std::string>::const_iterator const& end
                       , value_type& node
                       , value_type const& value
                       )
            : _key (key)
            , _end (end)
            , _node (node)
            , _value (value)
          {}

          value_type& operator() (structured_type& m) const
          {
            return (_key == _end) ? (_node = _value) : deeper (m);
          }

          template<typename T> value_type& operator() (T&) const
          {
            return (_key == _end)
              ? (_node = _value)
              : deeper (boost::get<structured_type> (_node = structured_type())
                       )
              ;
          }

        private:
          std::list<std::string>::const_iterator const& _key;
          std::list<std::string>::const_iterator const& _end;
          value_type& _node;
          value_type const& _value;

          value_type& deeper (structured_type& m) const
          {
            structured_type::iterator pos
              (std::find_if (m.begin(), m.end(), first_is (*_key)));

            if (pos == m.end())
            {
              pos = m.insert (m.end(), std::make_pair (*_key, value_type()));
            }

            value_type& v (pos->second);

            return boost::apply_visitor
              (visitor_poke (std::next (_key), _end, v, _value), v);
          }
        };
      }

      value_type& poke ( std::list<std::string>::const_iterator const& key
                       , std::list<std::string>::const_iterator const& end
                       , value_type& node
                       , value_type const& value
                       )
      {
        return boost::apply_visitor
          (visitor_poke (key, end, node, value), node);
      }
      value_type& poke ( std::list<std::string> const& path
                       , value_type& node
                       , value_type const& value
                       )
      {
        return poke (path.begin(), path.end(), node, value);
      }
      value_type& poke ( std::string const& path
                       , value_type& node
                       , value_type const& value
                       )
      {
        return poke (path::split (path), node, value);
      }
    }
  }
}
