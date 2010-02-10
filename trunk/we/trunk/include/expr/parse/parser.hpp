// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_PARSER_HPP
#define _EXPR_PARSE_PARSER_HPP

#include <expr/parse/action.hpp>
#include <expr/parse/node.hpp>

#include <expr/token/function.hpp>
#include <expr/token/prop.hpp>
#include <expr/token/tokenizer.hpp>
#include <expr/token/type.hpp>

#include <expr/eval/context.hpp>
#include <expr/eval/eval.hpp>

#include <expr/exception.hpp>

#include <stack>

#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace expr
{
  namespace parse
  {
    template<typename T>
    struct parser
    {
    private:
      typedef node::type<T> nd_t;
      typedef std::stack<nd_t> nd_stack_t;
      typedef std::stack<token::type> op_stack_t;
      nd_stack_t nd_stack;
      op_stack_t op_stack;

      void unary (const token::type & token, const unsigned int k)
      {
        if (nd_stack.empty())
          throw exception ("unary operator missing operand: " + show(token), k);

        nd_t * c = new nd_t(nd_stack.top()); nd_stack.pop();

        if (c->is_value)
          {
            nd_stack.push (nd_t(token::function::unary (token, c->value)));
            delete c;
          }
        else
          {
            nd_stack.push (nd_t (token, c));
          }
      }

      void binary (const token::type & token, const unsigned int k)
      {
        if (nd_stack.empty())
          throw exception ( "binary operator missing operand r: " + show(token)
                          , k
                          );

        nd_t * r = new nd_t(nd_stack.top()); nd_stack.pop();

        if (nd_stack.empty())
          throw exception ( "binary operator missing operand l: " + show(token)
                          , k
                          );

        nd_t * l = new nd_t(nd_stack.top()); nd_stack.pop();

        if (l->is_value && r->is_value)
          {
            nd_stack.push 
              (nd_t(token::function::binary (token, l->value, r->value)));
            delete l;
            delete r;
          }
        else
          {
            nd_stack.push (nd_t (token, l, r));
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
          case token::abs: unary (op_stack.top(), k); break;
          case token::com: binary (op_stack.top(), k); break;
          case token::rpr: op_stack.pop(); break;
          default: break;
          }
        op_stack.pop();
      }

      void parse ( const std::string & input
                 , const boost::function<nd_t (const std::string &)> & refnode
                 )
      {
        op_stack.push (token::eof);

        token::tokenizer<T> token (input);

        do
          {
            ++token;

            switch (*token)
              {
              case token::val: nd_stack.push (nd_t(token())); break;
              case token::ref: nd_stack.push (refnode(token.refname())); break;
              default:
                {
                ACTION:
                  action::type action (action::action (op_stack.top(), *token));

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

    public:
      parser (const std::string & input, const eval::context<T> & context)
      {
        parse ( input
              , boost::bind (eval::refnode_value<T>, boost::ref(context), _1)
              );
      }

      parser (const std::string & input)
      {
        parse (input, boost::bind (eval::refnode_name<T>, _1));
      }

      const nd_t & operator * (void) const { return nd_stack.top(); }

      const T eval (const eval::context<T> & context)
      {
        return eval::eval (this->operator * (), context);
      }
    };
  }
}

#endif
