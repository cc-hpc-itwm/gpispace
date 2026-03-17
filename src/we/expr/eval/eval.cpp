// Copyright (C) 2012-2016,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/expr/eval/eval.hpp>

#include <gspc/we/expr/token/assoc.hpp>
#include <gspc/we/expr/token/prop.hpp>
#include <gspc/we/expr/token/type.hpp>

#include <gspc/we/expr/exception.hpp>

#include <gspc/we/type/shared.hpp>
#include <gspc/we/type/value/function.hpp>
#include <gspc/we/type/value/show.hpp>

#include <gspc/we/expr/token/show.formatter.hpp>
#include <gspc/we/type/value/show.formatter.hpp>
#include <fmt/core.h>
#include <stdexcept>


  namespace gspc::we::expr::eval
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

        pnet::type::value::value_type operator () (parse::node::unary_t const& u) const
        {
          pnet::type::value::value_type c0 (::boost::apply_visitor (*this, u.child));

          return pnet::type::value::unary (u.token, c0);
        }

        pnet::type::value::value_type operator () (parse::node::binary_t const& b) const
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
          else if (b.token == token::_shared)
            {
              return we::type::shared
                { ::boost::apply_visitor (*this, b.r) // value
                , ::boost::get<std::string>           // cleanup place
                  ( ::boost::apply_visitor (*this, b.l)
                  )
                };
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

        pnet::type::value::value_type operator () (parse::node::ternary_t const& t) const
        {
          switch (t.token)
            {
            case token::_map_assign:
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
                    throw exception::eval::type_error
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
                              , token::show (t.token)
                              )
                };
            }
        }
      };
    }

    pnet::type::value::value_type eval (context& c, parse::node::type const& nd)
    {
      return ::boost::apply_visitor (visitor_eval (c), nd);
    }
  }
