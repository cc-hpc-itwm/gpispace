// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/parse/parser.hpp>

#include <we/expr/parse/action.hpp>
#include <we/expr/token/tokenizer.hpp>

#include <we/expr/token/prop.hpp>

#include <we/type/value/function.hpp>

#include <we/expr/eval/context.hpp>
#include <we/expr/eval/eval.hpp>

#include <fhg/util/show.hpp>

#include <boost/bind.hpp>

#include <sstream>
#include <sstream>
#include <iterator>

namespace expr
{
  namespace parse
  {
    parser::parser ( const std::string & input
                   , eval::context & context
                   , const bool& constant_folding
                   )
      : op_stack ()
      , nd_stack ()
      , tmp_stack ()
      , _constant_folding (constant_folding)
    {
      parse (input, boost::bind ( eval::refnode_value
                                , boost::ref(context)
                                , _1
                                )
            );
    }
    parser::parser ( const std::string & input
                   , const bool& constant_folding
                   )
      : op_stack ()
      , nd_stack ()
      , tmp_stack ()
      , _constant_folding (constant_folding)
    {
      parse (input, eval::refnode_name);
    }
    parser::parser ( const nd_stack_t & seq
                   , const bool& constant_folding
                   )
      : op_stack ()
      , nd_stack (seq)
      , tmp_stack ()
      , _constant_folding (constant_folding)
    {}

    void parser::add (const parser & other)
    {
      std::copy (other.begin(), other.end(), std::back_inserter(nd_stack));
    }

    value::type parser::eval_front (eval::context & context) const
    {
      return eval::eval (context, front());
    }

    bool parser::eval_front_bool (eval::context & context) const
    {
      return value::function::is_true(eval_front (context));
    }

    // get the already evaluated value, throws if entry is not an value
    const value::type & parser::get_front () const
    {
      return node::get (front());
    }

    bool parser::get_front_bool () const
    {
      return value::function::is_true(get_front ());
    }

    // evaluate the whole stack in order, return the last value
    value::type parser::eval_all (eval::context & context) const
    {
      value::type v;

      for (nd_const_it_t it (begin()); it != end(); ++it)
        {
          v = eval::eval (context, *it);
        }

      return v;
    }

    bool parser::eval_all_bool (eval::context & context) const
    {
      const value::type v (eval_all (context));

      return value::function::is_true(v);
    }

    value::type parser::eval_all() const
    {
      eval::context c;

      return eval_all (c);
    }

    void parser::rename ( const key_vec_t::value_type & from
                        , const key_vec_t::value_type & to
                        )
    {
      std::for_each (begin(), end(), boost::bind (node::rename, _1, from, to));
    }

    std::string parser::string (void) const
    {
      std::ostringstream s;

      std::copy (begin(), end(), std::ostream_iterator<nd_t> (s, ";"));

      return s.str();
    }

    void parser::unary (const token::type & token, const std::size_t k)
    {
      if (tmp_stack.empty())
        throw exception::parse::missing_operand (k);

      nd_t c (tmp_stack.back()); tmp_stack.pop_back();

      if (constant_folding() && node::is_value (c))
        {
          tmp_stack.push_back
            (boost::apply_visitor ( value::function::unary (token)
                                  , boost::get<value::type> (c)
                                  )
            );
        }
      else
        {
          tmp_stack.push_back (node::unary_t (token, c));
        }
    }

    void parser::binary (const token::type & token, const std::size_t k)
    {
      if (tmp_stack.empty())
        throw exception::parse::missing_operand (k, "left");

      nd_t r (tmp_stack.back()); tmp_stack.pop_back();

      if (tmp_stack.empty())
        throw exception::parse::missing_operand (k, "right");

      nd_t l (tmp_stack.back()); tmp_stack.pop_back();

      if (token::is_define (token) && not node::is_ref (l))
        {
          throw exception::parse::exception ( "left hand of "
                                            + fhg::util::show (token)
                                            + " must be reference name"
                                            , k
                                            );
        }

      if (constant_folding() && node::is_value(l) && node::is_value(r))
        {
          tmp_stack.push_back
            (boost::apply_visitor ( value::function::binary (token)
                                  , boost::get<value::type> (l)
                                  , boost::get<value::type> (r)
                                  )
            );
        }
      else
        {
          tmp_stack.push_back (node::binary_t (token, l, r));
        }
    }

    void parser::ternary (const token::type & token, const std::size_t k)
    {
      if (tmp_stack.empty())
        throw exception::parse::missing_operand (k, "first");

      nd_t t (tmp_stack.back()); tmp_stack.pop_back();

      if (tmp_stack.empty())
        throw exception::parse::missing_operand (k, "second");

      nd_t s (tmp_stack.back()); tmp_stack.pop_back();

      if (tmp_stack.empty())
        throw exception::parse::missing_operand (k, "third");

      nd_t f (tmp_stack.back()); tmp_stack.pop_back();

      tmp_stack.push_back (node::ternary_t (token, f, s, t));
    }

    void parser::ite (const std::size_t k)
    {
      if (tmp_stack.empty())
        throw exception::parse::missing_operand (k, "else expression");

      nd_t f (tmp_stack.back()); tmp_stack.pop_back();

      if (tmp_stack.empty())
        throw exception::parse::missing_operand (k, "then expression");

      nd_t t (tmp_stack.back()); tmp_stack.pop_back();

      if (tmp_stack.empty())
        throw exception::parse::missing_operand (k, "condition");

      nd_t c (tmp_stack.back()); tmp_stack.pop_back();

      if (constant_folding() && node::is_value(c))
        {
          if (value::function::is_true(boost::get<value::type> (c)))
            tmp_stack.push_back (t);
          else
            tmp_stack.push_back (f);
        }
      else
        {
          tmp_stack.push_back (node::ternary_t (token::_ite,  c, t, f));
        }
    }

    void parser::reduce (const std::size_t k)
    {
      switch (op_stack.top())
        {
        case token::_or:
        case token::_and: binary (op_stack.top(), k); break;
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
        case token::_set_is_element: binary (op_stack.top(), k); break;
        case token::_set_pop:
        case token::_set_top:
        case token::_set_empty:
        case token::_set_size: unary (op_stack.top(), k); break;
        case token::_substr:
        case token::mul:
        case token::div:
        case token::mod:
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
        case token::_len:
        case token::_tolong:
        case token::_todouble:
        case token::abs: unary (op_stack.top(), k); break;
        case token::rpr: op_stack.pop(); break;
        case token::define: binary (op_stack.top(), k); break;
        case token::_else: ite (k); break;
        default: break;
        }
      op_stack.pop();
    }

    void
    parser::parse ( const std::string& input
                  , const boost::function<nd_t (const key_vec_t &)> & refnode
                  )
    {
      std::string::const_iterator pos (input.begin());
      const std::string::const_iterator end (input.end());
      std::size_t k (0);

      op_stack.push (token::eof);

      token::tokenizer token (k, pos, end);

      while (pos != end)
        {
          do
            {
              ++token;

              switch (*token)
                {
                case token::val:
                  tmp_stack.push_back (token());
                  break;
                case token::ref:
                  tmp_stack.push_back (refnode(token.get_ref()));
                  break;
                default:
                  {
                  ACTION:
                    action::type action
                      (action::action (op_stack.top(), *token));

                    switch (action)
                      {
                      case action::reduce:
                        reduce(k);
                        goto ACTION;
                        break;
                      case action::shift:
                        op_stack.push (*token);
                        break;
                      case action::accept:
                        std::copy ( tmp_stack.begin()
                                  , tmp_stack.end()
                                  , std::back_inserter (nd_stack)
                                  );
                        tmp_stack.clear();
                        break;
                      default:
                        throw exception::parse::exception (fhg::util::show(action), k);
                      }
                    break;
                  }
                }
            }
          while (*token != token::eof);
        }
    }

    std::ostream & operator << (std::ostream & s, const parser & p)
    {
      std::copy ( p.begin()
                , p.end()
                , std::ostream_iterator<parser::nd_t> (s, ";\n")
                );

      return s;
    }

    std::string parse_result ( const std::string& input
                             , const bool& constant_folding
                             )
    {
      std::ostringstream oss;

      try
        {
          oss << parser (input, constant_folding) << std::endl;
        }
      catch (const exception::parse::exception& e)
        {
          std::string::const_iterator pos (input.begin());

          std::string white;

          for (unsigned int k (0); k < e.eaten; ++k, ++pos)
            {
              oss << *pos;

              if (*pos == '\n')
                {
                  white.clear();
                }
              else
                {
                  white += " ";
                }
            }
          oss << std::endl << white << "^" << std::endl;

          oss << e.what() << std::endl;
        }
      catch (const exception::eval::type_error& e)
        {
          oss << e.what() << std::endl;
        }
      catch (const exception::eval::divide_by_zero& e)
        {
          oss << e.what() << std::endl;
        }

      return oss.str();
    }
  }
}
