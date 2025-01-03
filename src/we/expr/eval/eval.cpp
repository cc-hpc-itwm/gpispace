// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/expr/eval/eval.hpp>

#include <we/expr/token/assoc.hpp>
#include <we/expr/token/prop.hpp>

#include <we/expr/exception.hpp>

#include <we/type/value/function.hpp>
#include <we/type/value/show.hpp>

#include <FMT/we/expr/token/show.hpp>
#include <FMT/we/type/value/show.hpp>
#include <fmt/core.h>
#include <stdexcept>

namespace expr
{
  namespace eval
  {
    namespace
    {
      class visitor_eval : public ::boost::static_visitor<pnet::type::value::value_type>
      {
      private:
        context& c;

      public:
        visitor_eval (context& _c) : c (_c) {}

        pnet::type::value::value_type operator() (pnet::type::value::value_type const& v) const
        {
          return v;
        }

        pnet::type::value::value_type operator() (std::list<std::string> const& key) const
        {
          return c.value (key);
        }

        pnet::type::value::value_type operator () (expr::parse::node::unary_t const& u) const
        {
          pnet::type::value::value_type c0 (::boost::apply_visitor (*this, u.child));

          return pnet::type::value::unary (u.token, c0);
        }

        pnet::type::value::value_type operator () (expr::parse::node::binary_t const& b) const
        {
          if (is_define (b.token))
            {
              pnet::type::value::value_type c1 (::boost::apply_visitor (*this, b.r));

              c.bind_and_discard_ref
                ( ::boost::get<std::list<std::string>>(b.l)
                , c1
                );

              return c1;
            }
          else if (is_or_boolean (b.token))
            {
              pnet::type::value::value_type c0 (::boost::apply_visitor (*this, b.l));

              if (!::boost::get<bool> (c0))
                {
                  pnet::type::value::value_type c1 (::boost::apply_visitor (*this, b.r));

                  return pnet::type::value::binary (b.token, c0, c1);
                }
              else
                {
                  return c0;
                }
            }
          else if (is_and_boolean (b.token))
            {
              pnet::type::value::value_type c0 (::boost::apply_visitor (*this, b.l));

              if (::boost::get<bool> (c0))
                {
                  pnet::type::value::value_type c1 (::boost::apply_visitor (*this, b.r));

                  return pnet::type::value::binary (b.token, c0, c1);
                }
              else
                {
                  return c0;
                }
            }
          else
            {
              if (  associativity::associativity (b.token)
                 == associativity::left
                 )
                {
                  pnet::type::value::value_type c0 (::boost::apply_visitor (*this, b.l));
                  pnet::type::value::value_type c1 (::boost::apply_visitor (*this, b.r));

                  return pnet::type::value::binary (b.token, c0, c1);
                }
              else
                {
                  pnet::type::value::value_type c1 (::boost::apply_visitor (*this, b.r));
                  pnet::type::value::value_type c0 (::boost::apply_visitor (*this, b.l));

                  return pnet::type::value::binary (b.token, c0, c1);
                }
            }

        }

        pnet::type::value::value_type operator () (expr::parse::node::ternary_t const& t) const
        {
          switch (t.token)
            {
            case expr::token::_map_assign:
              {
                pnet::type::value::value_type c0 (::boost::apply_visitor (*this, t.child0));
                pnet::type::value::value_type c1 (::boost::apply_visitor (*this, t.child1));
                pnet::type::value::value_type c2 (::boost::apply_visitor (*this, t.child2));

                try
                  {
                    using map_type =
                      std::map< pnet::type::value::value_type
                              , pnet::type::value::value_type
                              >;
                    auto& m (::boost::get<map_type> (c0));

                    m[c1] = c2;

                    return c0;
                  }
                catch (...)
                  {
                    throw expr::exception::eval::type_error
                      { fmt::format ( "map_assign ({}, {}, {})"
                                    , pnet::type::value::show (c0)
                                    , pnet::type::value::show (c1)
                                    , pnet::type::value::show (c2)
                                    )
                      };
                  }
              }
            default: throw std::runtime_error
                { fmt::format ( "eval-ternary ({})"
                              , expr::token::show (t.token)
                              )
                };
            }
        }
      };
    }

    pnet::type::value::value_type eval (context& c, expr::parse::node::type const& nd)
    {
      return ::boost::apply_visitor (visitor_eval (c), nd);
    }
  }
}
