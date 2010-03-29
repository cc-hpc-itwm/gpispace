// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_PARSER_HPP
#define _EXPR_PARSE_PARSER_HPP

#include <we/expr/parse/action.hpp>
#include <we/expr/parse/node.hpp>

#include <we/expr/token/function.hpp>
#include <we/expr/token/prop.hpp>
#include <we/expr/token/tokenizer.hpp>
#include <we/expr/token/type.hpp>

#include <we/expr/eval/context.hpp>
#include <we/expr/eval/eval.hpp>

#include <we/expr/exception.hpp>

#include <we/util/read.hpp>

#include <stack>
#include <deque>

#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace expr
{
  namespace parse
  {
    class missing_operand : public exception
    {
    public:
      missing_operand (const unsigned int k, const std::string & what)
        : exception ("missing " + what + " operand", k-1) {}
      missing_operand (const unsigned int k)
        : exception ("missing operand", k-1) {}
    };

    template< typename Key
            , typename Value
            , Key READ (const std::string &) = read<Key>
            >
    struct parser
    {
    private:
      typedef node::type<Key,Value> nd_t;
      typedef std::deque<nd_t> nd_stack_t;
      typedef std::stack<token::type> op_stack_t;
      nd_stack_t nd_stack;
      op_stack_t op_stack;

      void unary (const token::type & token, const unsigned int k)
      {
        if (nd_stack.empty())
          throw missing_operand (k);

        nd_t c (nd_stack.back()); nd_stack.pop_back();

        if (c.is_value())
          nd_stack.push_back (nd_t(token::function::unary (token, c.value)));
        else
          {
            typename nd_t::ptr_t ptr_c (new nd_t(c));

            nd_stack.push_back (nd_t (token, ptr_c));
          }
      }

      void binary (const token::type & token, const unsigned int k)
      {
        if (nd_stack.empty())
          throw missing_operand (k, "left");

        nd_t r (nd_t(nd_stack.back())); nd_stack.pop_back();

        if (nd_stack.empty())
          throw missing_operand (k, "right");

        nd_t l (nd_t(nd_stack.back())); nd_stack.pop_back();

        if (l.is_value() && r.is_value())
          nd_stack.push_back (nd_t (token::function::binary ( token
                                                            , l.value
                                                            , r.value
                                                            )
                                   )
                             );
        else
          {
            typename nd_t::ptr_t ptr_l (new nd_t (l));
            typename nd_t::ptr_t ptr_r (new nd_t (r));

            nd_stack.push_back (nd_t (token, ptr_l, ptr_r));
          }
      }

      void reduce (const unsigned int k)
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
          case token::mul:
          case token::div:
          case token::mod:
          case token::pow: binary (op_stack.top(), k); break;
          case token::neg:
          case token::fac: unary (op_stack.top(), k); break;
          case token::min:
          case token::max: binary (op_stack.top(), k); break;
          case token::_sin:
          case token::_cos:
          case token::_sqrt:
          case token::_log:
          case token::abs: unary (op_stack.top(), k); break;
          case token::com: binary (op_stack.top(), k); break;
          case token::rpr: op_stack.pop(); break;
          case token::define: binary (op_stack.top(), k); break;
          default: break;
          }
        op_stack.pop();
      }

      void parse ( const std::string input
                 , const boost::function<nd_t (const Key &)> & refnode
                 )
      {
        std::string::const_iterator pos (input.begin());
        const std::string::const_iterator end (input.end());
        unsigned int k (0);

        while (pos != end)
          {
            op_stack.push (token::eof);

            token::tokenizer<Key,Value,READ> token (k, pos, end);

            do
              {
                ++token;

                switch (*token)
                  {
                  case token::val:
                    nd_stack.push_back (nd_t(token()));
                    break;
                  case token::ref:
                    nd_stack.push_back (refnode(token.get_ref()));
                    break;
                  case token::define:
                    if (nd_stack.empty() || !nd_stack.back().is_ref())
                      throw exception ("left hand of " + show(*token) + " must be reference name", token.eaten());
                    op_stack.push (*token);
                    break;
                  default:
                    {
                    ACTION:
                      action::type action
                        (action::action (op_stack.top(), *token));

                      switch (action)
                        {
                        case action::reduce:
                          reduce(token.eaten());
                          goto ACTION;
                          break;
                        case action::shift:
                          op_stack.push (*token);
                          break;
                        case action::accept:
                          break;
                        default:
                          throw exception (show(action), token.eaten());
                        }
                      break;
                    }
                  }
              }
            while (*token != token::eof);
          }
      }

    public:
      parser (const std::string & input, eval::context<Key,Value> & context)
      {
        parse ( input, boost::bind ( eval::refnode_value<Key,Value>
                                   , boost::ref(context)
                                   , _1
                                   )
              );
      }

      parser (const std::string & input)
      {
        parse (input, boost::bind (eval::refnode_name<Key,Value>, _1));
      }

      bool empty (void) const
      {
        return nd_stack.empty();
      }

      void pop (void)
      {
        nd_stack.pop_front();
      }

      const nd_t & expr (void) const
      {
        return nd_stack.front();
      }

      const Value eval (eval::context<Key,Value> & context) const
      {
        return eval::eval (expr(), context);
      }

      bool eval_bool (eval::context<Key,Value> & context) const
      {
        return !token::function::is_zero (eval (context));
      }

      const Value & get () const
      {
        return node::get (expr());
      }

      bool get_bool () const 
      {
        return !token::function::is_zero (get ());
      }
    };
  }
}

#endif
