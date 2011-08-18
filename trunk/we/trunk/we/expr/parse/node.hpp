// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_NODE_HPP
#define _EXPR_PARSE_NODE_HPP

#include <we/expr/token/type.hpp>
#include <we/expr/token/prop.hpp>
#include <we/expr/token/tokenizer.hpp>
#include <we/expr/exception.hpp>

#include <we/type/value.hpp>
#include <we/type/value/show.hpp>

#include <stdexcept>
#include <iostream>
#include <vector>

#include <boost/variant.hpp>

namespace expr
{
  namespace parse
  {
    namespace node
    {
      struct unary_t;
      struct binary_t;
      struct ternary_t;

      typedef expr::token::key_vec_t key_vec_t;

      typedef boost::variant < value::type
                             , key_vec_t
                             , boost::recursive_wrapper<unary_t>
                             , boost::recursive_wrapper<binary_t>
                             , boost::recursive_wrapper<ternary_t>
                             > type;

      std::ostream & operator << (std::ostream &, const type &);

      struct unary_t
      {
        token::type token;
        type child;

        unary_t (const token::type & _token, const type & _child)
          : token (_token), child (_child)
        {}
      };

      struct binary_t
      {
        token::type token;
        type l;
        type r;

        binary_t (const token::type & _token, const type & _l, const type & _r)
          : token (_token), l (_l), r (_r)
        {}
      };

      struct ternary_t
      {
        token::type token;
        type child0;
        type child1;
        type child2;

        ternary_t ( const token::type & _token
                  , const type & _child0
                  , const type & _child1
                  , const type & _child2
                  )
          : token (_token), child0 (_child0), child1 (_child1), child2 (_child2)
        {}
      };

      // ******************************************************************* //

      namespace visitor
      {
        class is_value : public boost::static_visitor<bool>
        {
        public:
          bool operator () (const value::type &) const { return true; }

          template<typename T>
          bool operator () (const T &) const { return false; }
        };

        // ***************************************************************** //

        class get_value : public boost::static_visitor<const value::type &>
        {
        public:
          const value::type & operator () (const value::type & v) const
          {
            return v;
          }

          template<typename T>
          const value::type & operator () (const T &) const
          {
            throw exception::eval::type_error ("get: node is not an value");
          }
        };

        // ***************************************************************** //

        class is_ref : public boost::static_visitor<bool>
        {
        public:
          bool operator () (const key_vec_t &) const { return true; }

          template<typename T>
          bool operator () (const T &) const { return false; }
        };

        // ***************************************************************** //

        class show : public boost::static_visitor<void>
        {
        private:
          std::ostream & s;

        public:
          show (std::ostream & _s) : s (_s) {}

          void operator () (const value::type & v) const
          {
            boost::apply_visitor (value::visitor::show (s), v);
          }

          void operator () (const key_vec_t & key) const
          {
            s << "${" << token::show_key_vec (key) << "}";
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
              case token::_ite:
                s << "(" << token::_if << t.child0
                         << token::_then << t.child1
                         << token::_else << t.child2
                         << token::_endif
                  << ")"
                  ;
              case token::_map_assign:
                s << token::_map_assign
                  << "(" << t.child0
                  << "," << t.child1
                  << "," << t.child2
                  << ")"
                  ;
                break;
              default:
                throw exception::strange ("unknown ternary token");
              }
          }
        };
      }

      inline std::ostream & operator << (std::ostream & s, const type & node)
      {
        boost::apply_visitor (visitor::show (s), node);

        return s;
      }

      inline const value::type & get (const type & node)
      {
        return boost::apply_visitor (visitor::get_value(), node);
      }

      // ******************************************************************* //

      namespace visitor
      {
        class rename : public boost::static_visitor<void>
        {
        private:
          const key_vec_t::value_type from;
          const key_vec_t::value_type to;

        public:
          rename ( const key_vec_t::value_type & _from
                 , const key_vec_t::value_type & _to
                 )
            : from (_from), to (_to)
          {}

          void operator () (value::type &) const
          {
            return;
          }

          void operator () (key_vec_t & v) const
          {
            if (v.size() > 0 && v[0] == from)
              {
                v[0] = to;
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
        };
      }

      inline void rename ( type & t
                         , const key_vec_t::value_type & from
                         , const key_vec_t::value_type & to
                         )
      {
        boost::apply_visitor (visitor::rename (from, to), t);
      }
    }
  }
}

#endif
