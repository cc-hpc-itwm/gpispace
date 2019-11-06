
#include <we/exception.hpp>
#include <we/expr/eval/context.hpp>
#include <we/expr/eval/eval.hpp>
#include <we/expr/parse/action.hpp>
#include <we/expr/parse/parser.hpp>
#include <we/expr/token/prop.hpp>
#include <we/expr/token/tokenizer.hpp>
#include <we/type/value/function.hpp>

#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <sstream>

namespace expr
{
  namespace parse
  {
    parser::parser (std::string const& input)
      : op_stack()
      , nd_stack()
      , tmp_stack()
    {
      parse (input);
    }
    parser::parser (nd_stack_t const& seq)
      : op_stack()
      , nd_stack (seq)
      , tmp_stack()
    {}

    // evaluate the whole stack in order, return the last value
    pnet::type::value::value_type
      parser::eval_all (eval::context& context) const
    {
      return std::accumulate
        ( nd_stack.begin(), nd_stack.end()
        , pnet::type::value::value_type()
        , [&context] ( pnet::type::value::value_type&
                     , node::type const& node
                     )
          {
            return eval::eval (context, node);
          }
        );
    }

    pnet::type::value::value_type parser::eval_all() const
    {
      eval::context UNUSED_context;

      return eval_all (UNUSED_context);
    }

    bool parser::is_const_true() const
    {
      //! \todo more intelligent check, e.g. identify ${a} == ${a}
      try
      {
        return boost::get<bool> (eval_all());
      }
      catch (pnet::exception::missing_binding const&)
      {
        return false;
      }
    }


    void parser::rename (std::string const& from, std::string const& to)
    {
      std::for_each ( begin(), end()
                    , std::bind (node::rename, std::placeholders::_1, from, to)
                    );
    }

    node::KeyRoots parser::key_roots() const
    {
      node::KeyRoots roots;

      std::for_each ( begin(), end()
                    , [&] (nd_t const& node)
                      {
                        node::collect_key_roots (node, roots);
                      }
                    );

      return roots;
    }

    std::string parser::string() const
    {
      std::ostringstream s;

      std::copy (begin(), end(), std::ostream_iterator<nd_t> (s, ";"));

      return s.str();
    }

    void parser::unary (token::type const& token, std::size_t const k)
    {
      if (tmp_stack.empty())
      {
        throw exception::parse::missing_operand (k);
      }

      nd_t c (tmp_stack.back()); tmp_stack.pop_back();

      if (node::is_value (c))
      {
        tmp_stack.emplace_back
          (pnet::type::value::unary (token, node::get (c)));
      }
      else
      {
        tmp_stack.emplace_back (node::unary_t (token, c));
      }
    }

    void parser::binary (token::type const& token, std::size_t const k)
    {
      if (tmp_stack.empty())
      {
        throw exception::parse::missing_operand (k, "left");
      }

      nd_t r (tmp_stack.back()); tmp_stack.pop_back();

      if (tmp_stack.empty())
      {
        throw exception::parse::missing_operand (k, "right");
      }

      nd_t l (tmp_stack.back()); tmp_stack.pop_back();

      if (token::is_define (token) && not node::is_ref (l))
      {
        throw exception::parse::exception
          ( "left hand of "
          + boost::lexical_cast<std::string> (token)
          + " must be reference name"
          , k
          );
      }

      if (node::is_value(l) && node::is_value(r))
      {
        tmp_stack.emplace_back ( pnet::type::value::binary
                                 ( token
                                 , node::get (l)
                                 , node::get (r)
                                 )
                               );
      }
      else
      {
        tmp_stack.emplace_back (node::binary_t (token, l, r));
      }
    }

    void parser::ternary (token::type const& token, std::size_t const k)
    {
      if (tmp_stack.empty())
      {
        throw exception::parse::missing_operand (k, "first");
      }

      nd_t t (tmp_stack.back()); tmp_stack.pop_back();

      if (tmp_stack.empty())
      {
        throw exception::parse::missing_operand (k, "second");
      }

      nd_t s (tmp_stack.back()); tmp_stack.pop_back();

      if (tmp_stack.empty())
      {
        throw exception::parse::missing_operand (k, "third");
      }

      nd_t f (tmp_stack.back()); tmp_stack.pop_back();

      tmp_stack.emplace_back (node::ternary_t (token, f, s, t));
    }

    void parser::reduce (std::size_t const k)
    {
      switch (op_stack.top())
      {
      case token::_or_boolean:
      case token::_or_integral:
      case token::_and_boolean:
      case token::_and_integral: binary (op_stack.top(), k); break;
      case token::_not: unary (op_stack.top(), k); break;
      case token::lt:
      case token::le:
      case token::gt:
      case token::ge:
      case token::ne:
      case token::eq:
      case token::add:
      case token::sub:
      case token::_bitset_insert:
      case token::_bitset_delete:
      case token::_bitset_is_element:
      case token::_bitset_or:
      case token::_bitset_and:
      case token::_bitset_xor: binary (op_stack.top(), k); break;
      case token::_bitset_tohex:
      case token::_bitset_fromhex:
      case token::_bitset_count: unary (op_stack.top(), k); break;
      case token::_stack_empty:
      case token::_stack_top: unary (op_stack.top(), k); break;
      case token::_stack_push: binary (op_stack.top(), k); break;
      case token::_stack_pop:
      case token::_stack_size: unary (op_stack.top(), k); break;
      case token::_stack_join: binary (op_stack.top(), k); break;
      case token::_map_assign: ternary (op_stack.top(), k); break;
      case token::_map_unassign:
      case token::_map_is_assigned:
      case token::_map_get_assignment: binary (op_stack.top(), k); break;
      case token::_map_size:
      case token::_map_empty: unary (op_stack.top(),k); break;
      case token::_set_insert:
      case token::_set_erase:
      case token::_set_is_subset:
      case token::_set_is_element: binary (op_stack.top(), k); break;
      case token::_set_pop:
      case token::_set_top:
      case token::_set_empty:
      case token::_set_size: unary (op_stack.top(), k); break;
      case token::mul:
      case token::div:
      case token::divint:
      case token::modint:
      case token::_pow:
      case token::_powint: binary (op_stack.top(), k); break;
      case token::neg: unary (op_stack.top(), k); break;
      case token::min:
      case token::max: binary (op_stack.top(), k); break;
      case token::_floor:
      case token::_ceil:
      case token::_round:
      case token::_sin:
      case token::_cos:
      case token::_sqrt:
      case token::_log:
      case token::_toint:
      case token::_tolong:
      case token::_touint:
      case token::_toulong:
      case token::_tofloat:
      case token::_todouble:
      case token::abs: unary (op_stack.top(), k); break;
      case token::rpr: op_stack.pop(); break;
      case token::define: binary (op_stack.top(), k); break;
      default: break;
      }
      op_stack.pop();
    }

    void parser::parse (std::string const& input)
    {
      op_stack.push (token::eof);

      fhg::util::parse::position_string pos (input);
      token::tokenizer token (pos);

      while (!pos.end())
      {
        do
        {
          ++token;

          switch (token.token())
          {
          case token::val:
            tmp_stack.emplace_back (token.value());
            break;
          case token::ref:
            tmp_stack.emplace_back (token.get_ref());
            break;
          default:
            {
            ACTION:
              action::type const action
                (action::action (op_stack.top(), token.token()));

              switch (action)
              {
              case action::reduce:
                reduce (token.pos().eaten());
                goto ACTION;
              case action::shift:
                op_stack.push (token.token());
                break;
              case action::accept:
                std::copy ( tmp_stack.begin()
                          , tmp_stack.end()
                          , std::back_inserter (nd_stack)
                          );
                tmp_stack.clear();
                break;
              default:
                throw exception::parse::exception
                  ( boost::lexical_cast<std::string> (action)
                  , token.pos().eaten()
                  );
              }
              break;
            }
          }
        }
        while (token.token() != token::eof);
      }
    }
  }
}
