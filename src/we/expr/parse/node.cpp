// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <we/expr/parse/node.hpp>

#include <we/expr/token/prop.hpp>

#include <we/expr/exception.hpp>

#include <we/type/value/show.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace expr
{
  namespace parse
  {
    namespace node
    {
      unary_t::unary_t (const token::type & _token, const type & _child)
        : token (_token), child (_child)
      {}

      binary_t::binary_t ( const token::type & _token
                         , const type & _l
                         , const type & _r
                         )
          : token (_token), l (_l), r (_r)
      {}

      ternary_t::ternary_t ( const token::type & _token
                           , const type & _child0
                           , const type & _child1
                           , const type & _child2
                           )
        : token (_token), child0 (_child0), child1 (_child1), child2 (_child2)
      {}

      namespace {
        std::string show_key_vec (const Key& key_vec)
        {
          std::string s;

          for (const std::string& k : key_vec)
          {
            if (!s.empty())
            {
              s += ".";
            }
            s += k;
          }

          return s;
        }
      }

      namespace
      {
        class visitor_show : public boost::static_visitor<void>
        {
        private:
          std::ostream & s;

        public:
          visitor_show (std::ostream & _s) : s (_s) {}

          void operator () (const pnet::type::value::value_type & v) const
          {
            s << pnet::type::value::show (v);
          }

          void operator () (const Key& key) const
          {
            s << "${" << show_key_vec (key) << "}";
          }

          void operator () (const unary_t & u) const
          {
            s << u.token << "(" << u.child << ")";
          }

          void operator () (const binary_t & b) const
          {
            if (token::is_prefix (b.token))
              {
                s << b.token << "(" << b.l << ", " <<  b.r << ")";
              }
            else
              {
                s << "(" << b.l << b.token << b.r << ")";
              }
          }

          void operator () (const ternary_t & t) const
          {
            switch (t.token)
              {
              case token::_map_assign:
                s << token::_map_assign
                  << "(" << t.child0
                  << "," << t.child1
                  << "," << t.child2
                  << ")"
                  ;
                break;
              default: throw std::runtime_error
                  (( boost::format ("show_parse_node_ternary (%1%)")
                   % expr::token::show (t.token)
                   ).str()
                  );
              }
          }
        };
      }

      std::ostream& operator << (std::ostream & s, const type & node)
      {
        boost::apply_visitor (visitor_show (s), node);

        return s;
      }

      namespace
      {
        class visitor_get_value
          : public boost::static_visitor<const pnet::type::value::value_type &>
        {
        public:
          const pnet::type::value::value_type & operator () (const pnet::type::value::value_type & v) const
          {
            return v;
          }
          template<typename T>
          const pnet::type::value::value_type & operator () (const T &) const
          {
            throw exception::eval::type_error ("get: node is not an value");
          }
        };
      }

      const pnet::type::value::value_type& get (const type & node)
      {
        return boost::apply_visitor (visitor_get_value(), node);
      }

      namespace
      {
        class visitor_is_value : public boost::static_visitor<bool>
        {
        public:
          bool operator () (const pnet::type::value::value_type &) const { return true; }
          bool operator () (const Key&) const { return false; }
          bool operator () (const unary_t &) const { return false; }
          bool operator () (const binary_t &) const { return false; }
          bool operator () (const ternary_t &) const { return false; }
        };
      }

      bool is_value (const type& node)
      {
        return boost::apply_visitor (visitor_is_value(), node);
      }

      namespace
      {
        class visitor_is_ref : public boost::static_visitor<bool>
        {
        public:
          bool operator () (const pnet::type::value::value_type &) const { return false; }
          bool operator () (const Key&) const { return true; }
          bool operator () (const unary_t &) const { return false; }
          bool operator () (const binary_t &) const { return false; }
          bool operator () (const ternary_t &) const { return false; }
        };
      }

      bool is_ref (const type& node)
      {
        return boost::apply_visitor (visitor_is_ref(), node);
      }

      namespace
      {
        class visitor_rename : public boost::static_visitor<void>
        {
        public:
          visitor_rename (const std::string& _from, const std::string& _to)
            : from (_from), to (_to)
          {}

          void operator () (pnet::type::value::value_type &) const
          {
            return;
          }

          void operator () (Key& v) const
          {
            if (v.size() > 0 && *v.begin() == from)
              {
                *v.begin() = to;
              }
          }

          void operator () (unary_t & u) const
          {
            boost::apply_visitor (*this, u.child);
          }

          void operator () (binary_t & b) const
          {
            boost::apply_visitor (*this, b.l);
            boost::apply_visitor (*this, b.r);
          }

          void operator () (ternary_t & t) const
          {
            boost::apply_visitor (*this, t.child0);
            boost::apply_visitor (*this, t.child1);
            boost::apply_visitor (*this, t.child2);
          }

        private:
          const std::string from;
          const std::string to;
        };
      }

      void rename (type& t, const std::string& from, const std::string& to)
      {
        boost::apply_visitor (visitor_rename (from, to), t);
      }

      namespace
      {
        struct visitor_collect_key_roots : boost::static_visitor<void>
        {
          visitor_collect_key_roots (KeyRoots& roots) : _roots (roots) {}

          void operator() (pnet::type::value::value_type const&) const
          {
            return;
          }
          void operator() (Key const& key) const
          {
            if (key.empty())
            {
              throw std::invalid_argument ("collect_key_roots: empty key");
            }

            _roots.emplace (key.front());
          }

          void operator() (unary_t const& u) const
          {
            boost::apply_visitor (*this, u.child);
          }

          void operator() (binary_t const& b) const
          {
            boost::apply_visitor (*this, b.l);
            boost::apply_visitor (*this, b.r);
          }

          void operator() (ternary_t const& t) const
          {
            boost::apply_visitor (*this, t.child0);
            boost::apply_visitor (*this, t.child1);
            boost::apply_visitor (*this, t.child2);
          }

        private:
          KeyRoots& _roots;
        };
      }

      void collect_key_roots (type const& node, KeyRoots& roots)
      {
        boost::apply_visitor (visitor_collect_key_roots (roots), node);
      }
    }
  }
}
