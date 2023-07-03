// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
      unary_t::unary_t (token::type const& _token, type const& _child)
        : token (_token), child (_child)
      {}

      binary_t::binary_t ( token::type const& _token
                         , type const& _l
                         , type const& _r
                         )
          : token (_token), l (_l), r (_r)
      {}

      ternary_t::ternary_t ( token::type const& _token
                           , type const& _child0
                           , type const& _child1
                           , type const& _child2
                           )
        : token (_token), child0 (_child0), child1 (_child1), child2 (_child2)
      {}

      namespace {
        std::string show_key_vec (Key const& key_vec)
        {
          std::string s;

          for (std::string const& k : key_vec)
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
        class visitor_show : public ::boost::static_visitor<void>
        {
        private:
          std::ostream & s;

        public:
          visitor_show (std::ostream & _s) : s (_s) {}

          void operator () (pnet::type::value::value_type const& v) const
          {
            s << pnet::type::value::show (v);
          }

          void operator () (Key const& key) const
          {
            s << "${" << show_key_vec (key) << "}";
          }

          void operator () (unary_t const& u) const
          {
            s << u.token << "(" << u.child << ")";
          }

          void operator () (binary_t const& b) const
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

          void operator () (ternary_t const& t) const
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
                  (( ::boost::format ("show_parse_node_ternary (%1%)")
                   % expr::token::show (t.token)
                   ).str()
                  );
              }
          }
        };
      }

      std::ostream& operator << (std::ostream & s, type const& node)
      {
        ::boost::apply_visitor (visitor_show (s), node);

        return s;
      }

      namespace
      {
        class visitor_get_value
          : public ::boost::static_visitor<pnet::type::value::value_type const&>
        {
        public:
          pnet::type::value::value_type const& operator () (pnet::type::value::value_type const& v) const
          {
            return v;
          }
          template<typename T>
          pnet::type::value::value_type const& operator () (T const&) const
          {
            throw exception::eval::type_error ("get: node is not an value");
          }
        };
      }

      pnet::type::value::value_type const& get (type const& node)
      {
        return ::boost::apply_visitor (visitor_get_value(), node);
      }

      namespace
      {
        class visitor_is_value : public ::boost::static_visitor<bool>
        {
        public:
          bool operator () (pnet::type::value::value_type const&) const { return true; }
          bool operator () (Key const&) const { return false; }
          bool operator () (unary_t const&) const { return false; }
          bool operator () (binary_t const&) const { return false; }
          bool operator () (ternary_t const&) const { return false; }
        };
      }

      bool is_value (type const& node)
      {
        return ::boost::apply_visitor (visitor_is_value(), node);
      }

      namespace
      {
        class visitor_is_ref : public ::boost::static_visitor<bool>
        {
        public:
          bool operator () (pnet::type::value::value_type const&) const { return false; }
          bool operator () (Key const&) const { return true; }
          bool operator () (unary_t const&) const { return false; }
          bool operator () (binary_t const&) const { return false; }
          bool operator () (ternary_t const&) const { return false; }
        };
      }

      bool is_ref (type const& node)
      {
        return ::boost::apply_visitor (visitor_is_ref(), node);
      }

      namespace
      {
        class visitor_rename : public ::boost::static_visitor<void>
        {
        public:
          visitor_rename (std::string const& _from, std::string const& _to)
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
            ::boost::apply_visitor (*this, u.child);
          }

          void operator () (binary_t & b) const
          {
            ::boost::apply_visitor (*this, b.l);
            ::boost::apply_visitor (*this, b.r);
          }

          void operator () (ternary_t & t) const
          {
            ::boost::apply_visitor (*this, t.child0);
            ::boost::apply_visitor (*this, t.child1);
            ::boost::apply_visitor (*this, t.child2);
          }

        private:
          const std::string from;
          const std::string to;
        };
      }

      void rename (type& t, std::string const& from, std::string const& to)
      {
        ::boost::apply_visitor (visitor_rename (from, to), t);
      }

      namespace
      {
        struct visitor_collect_key_roots : ::boost::static_visitor<void>
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
            ::boost::apply_visitor (*this, u.child);
          }

          void operator() (binary_t const& b) const
          {
            ::boost::apply_visitor (*this, b.l);
            ::boost::apply_visitor (*this, b.r);
          }

          void operator() (ternary_t const& t) const
          {
            ::boost::apply_visitor (*this, t.child0);
            ::boost::apply_visitor (*this, t.child1);
            ::boost::apply_visitor (*this, t.child2);
          }

        private:
          KeyRoots& _roots;
        };
      }

      void collect_key_roots (type const& node, KeyRoots& roots)
      {
        ::boost::apply_visitor (visitor_collect_key_roots (roots), node);
      }
    }
  }
}
