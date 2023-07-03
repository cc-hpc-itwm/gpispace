// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/value/path/split.hpp>
#include <we/type/value/poke.hpp>

#include <algorithm>
#include <iterator>
#include <list>
#include <string>
#include <utility>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        struct first_is
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

        class visitor_poke : public ::boost::static_visitor<value_type&>
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
              : deeper (::boost::get<structured_type> (_node = structured_type())
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
            auto pos
              (std::find_if (m.begin(), m.end(), first_is (*_key)));

            if (pos == m.end())
            {
              pos = m.insert (m.end(), std::make_pair (*_key, value_type()));
            }

            value_type& v (pos->second);

            return ::boost::apply_visitor
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
        return ::boost::apply_visitor
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
