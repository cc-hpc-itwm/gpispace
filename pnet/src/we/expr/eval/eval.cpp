// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/eval/eval.hpp>

#include <we/expr/token/assoc.hpp>
#include <we/expr/token/prop.hpp>

#include <we/expr/exception.hpp>

#include <we/type/value.hpp>
#include <we/type/value/get.hpp>
#include <we/type/value/function.hpp>

#include <we2/type/value/show.hpp>

#include <boost/format.hpp>

namespace expr
{
  namespace eval
  {
    namespace
    {
      class visitor_eval : public boost::static_visitor<pnet::type::value::value_type>
      {
      private:
        context& c;

      public:
        visitor_eval (context& _c) : c (_c) {}

        pnet::type::value::value_type operator() (const pnet::type::value::value_type& v) const
        {
          return v;
        }

        pnet::type::value::value_type operator() (const std::list<std::string>& key) const
        {
          return c.value (key);
        }

        pnet::type::value::value_type operator () (const expr::parse::node::unary_t& u) const
        {
          pnet::type::value::value_type c0 (boost::apply_visitor (*this, u.child));

          return value::function::unary (u.token, c0);
        }

        pnet::type::value::value_type operator () (const expr::parse::node::binary_t& b) const
        {
          if (is_define (b.token))
            {
              pnet::type::value::value_type c1 (boost::apply_visitor (*this, b.r));

              c.bind_and_discard_ref
                ( boost::get<const std::list<std::string>&>(b.l)
                , c1
                );

              return c1;
            }
          else if (is_or (b.token))
            {
              pnet::type::value::value_type c0 (boost::apply_visitor (*this, b.l));

              if (!value::function::is_true (c0))
                {
                  pnet::type::value::value_type c1 (boost::apply_visitor (*this, b.r));

                  return value::function::binary (b.token, c0, c1);
                }
              else
                {
                  return c0;
                }
            }
          else if (is_and (b.token))
            {
              pnet::type::value::value_type c0 (boost::apply_visitor (*this, b.l));

              if (value::function::is_true (c0))
                {
                  pnet::type::value::value_type c1 (boost::apply_visitor (*this, b.r));

                  return value::function::binary (b.token, c0, c1);
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
                  pnet::type::value::value_type c0 (boost::apply_visitor (*this, b.l));
                  pnet::type::value::value_type c1 (boost::apply_visitor (*this, b.r));

                  return value::function::binary (b.token, c0, c1);
                }
              else
                {
                  pnet::type::value::value_type c1 (boost::apply_visitor (*this, b.r));
                  pnet::type::value::value_type c0 (boost::apply_visitor (*this, b.l));

                  return value::function::binary (b.token, c0, c1);
                }
            }

        }

        pnet::type::value::value_type operator () (const expr::parse::node::ternary_t& t) const
        {
          switch (t.token)
            {
            case expr::token::_map_assign:
              {
                pnet::type::value::value_type c0 (boost::apply_visitor (*this, t.child0));
                pnet::type::value::value_type c1 (boost::apply_visitor (*this, t.child1));
                pnet::type::value::value_type c2 (boost::apply_visitor (*this, t.child2));

                try
                  {
                    typedef std::map< pnet::type::value::value_type
                                    , pnet::type::value::value_type
                                    > map_type;
                    map_type& m (boost::get<map_type&> (c0));

                    m[c1] = c2;

                    return c0;
                  }
                catch (...)
                  {
                    throw expr::exception::eval::type_error
                      (( boost::format ("map_assign (%1%, %2%, %3%)")
                       % pnet::type::value::show (c0)
                       % pnet::type::value::show (c1)
                       % pnet::type::value::show (c2)
                       ).str()
                      );
                  }
              }
            default:
              throw expr::exception::strange ("unknown ternary token");
            }
        }
      };
    }

    pnet::type::value::value_type eval (context& c, const expr::parse::node::type& nd)
    {
      return boost::apply_visitor (visitor_eval (c), nd);
    }
  }
}
