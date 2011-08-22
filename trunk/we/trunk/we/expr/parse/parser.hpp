// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_PARSER_HPP
#define _EXPR_PARSE_PARSER_HPP

#include <we/expr/parse/action.hpp>
#include <we/expr/parse/node.hpp>

#include <we/expr/token/prop.hpp>
#include <we/expr/token/tokenizer.hpp>
#include <we/expr/token/type.hpp>

#include <we/type/value.hpp>
#include <we/type/value/function.hpp>

#include <we/expr/eval/context.hpp>
#include <we/expr/eval/eval.hpp>

#include <we/expr/exception.hpp>

#include <fhg/util/read.hpp>
#include <fhg/util/show.hpp>

#include <stack>
#include <list>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <iostream>

namespace expr
{
  namespace exception
  {
    namespace parse
    {
      class missing_operand : public exception
      {
      public:
        missing_operand (const std::size_t k, const std::string & what)
          : exception ("missing " + what + " operand", k-1) {}
        missing_operand (const std::size_t k)
          : exception ("missing operand", k-1) {}
      };
    }
  }

  namespace parse
  {
    struct parser
    {
    public:
      typedef node::key_vec_t key_vec_t;
      typedef node::type nd_t;
      typedef std::list<nd_t> nd_stack_t;

      // iterate through the entries
      typedef nd_stack_t::const_iterator nd_const_it_t;
      typedef nd_stack_t::iterator nd_it_t;

    private:
      typedef std::stack<token::type> op_stack_t;
      op_stack_t op_stack;
      nd_stack_t nd_stack;

      nd_it_t begin () { return nd_stack.begin(); }
      nd_it_t end () { return nd_stack.end(); }

    public:
      nd_const_it_t begin () const { return nd_stack.begin(); }
      nd_const_it_t end () const { return nd_stack.end(); }

    private:
      void unary (const token::type & token, const std::size_t k)
      {
        if (nd_stack.empty())
          throw exception::parse::missing_operand (k);

        nd_t c (nd_stack.back()); nd_stack.pop_back();

        if (boost::apply_visitor (node::visitor::is_value(), c))
          nd_stack.push_back
            (nd_t (boost::apply_visitor ( value::function::unary (token)
                                        , boost::get<value::type> (c)
                                        )
                  )
            );
        else
          {
            nd_stack.push_back (node::unary_t (token, c));
          }
      }

      void binary (const token::type & token, const std::size_t k)
      {
        if (nd_stack.empty())
          throw exception::parse::missing_operand (k, "left");

        nd_t r (nd_t(nd_stack.back())); nd_stack.pop_back();

        if (nd_stack.empty())
          throw exception::parse::missing_operand (k, "right");

        nd_t l (nd_t(nd_stack.back())); nd_stack.pop_back();

        if (  boost::apply_visitor (node::visitor::is_value(), l)
           && boost::apply_visitor (node::visitor::is_value(), r)
           )
          nd_stack.push_back
            (nd_t (boost::apply_visitor ( value::function::binary (token)
                                        , boost::get<value::type> (l)
                                        , boost::get<value::type> (r)
                                        )
                  )
            );
        else
          {
            nd_stack.push_back (node::binary_t (token, l, r));
          }
      }

      void ternary (const token::type & token, const std::size_t k)
      {
        if (nd_stack.empty())
          throw exception::parse::missing_operand (k, "first");

        nd_t t (nd_t(nd_stack.back())); nd_stack.pop_back();

        if (nd_stack.empty())
          throw exception::parse::missing_operand (k, "second");

        nd_t s (nd_t(nd_stack.back())); nd_stack.pop_back();

        if (nd_stack.empty())
          throw exception::parse::missing_operand (k, "third");

        nd_t f (nd_t(nd_stack.back())); nd_stack.pop_back();

        nd_stack.push_back (node::ternary_t (token, f, s, t));
      }

      void ite (const std::size_t k)
      {
        if (nd_stack.empty())
          throw exception::parse::missing_operand (k, "else expression");

        nd_t f (nd_t(nd_stack.back())); nd_stack.pop_back();

        if (nd_stack.empty())
          throw exception::parse::missing_operand (k, "then expression");

        nd_t t (nd_t(nd_stack.back())); nd_stack.pop_back();

        if (nd_stack.empty())
          throw exception::parse::missing_operand (k, "condition");

        nd_t c (nd_t(nd_stack.back())); nd_stack.pop_back();

        if (boost::apply_visitor (node::visitor::is_value(), c))
          {
            if (value::function::is_true(boost::get<value::type> (c)))
              nd_stack.push_back (t);
            else
              nd_stack.push_back (f);
          }
        else
          {
            nd_stack.push_back (node::ternary_t (token::_ite,  c, t, f));
          }
      }

      void reduce (const std::size_t k)
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
          case token::_bitset_is_element: binary (op_stack.top(), k); break;
          case token::_stack_empty:
          case token::_stack_top: unary (op_stack.top(), k); break;
          case token::_stack_push: binary (op_stack.top(), k); break;
          case token::_stack_pop:
          case token::_stack_size: unary (op_stack.top(), k); break;
          case token::_stack_join: binary (op_stack.top(), k); break;
          case token::_map_assign: ternary (op_stack.top(), k); break;
          case token::_map_unassign:
          case token::_map_is_assigned:
          case token::_map_get_assignment:
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
      parse ( const std::string input
            , const boost::function<nd_t (const key_vec_t &)> & refnode
            )
      {
        std::string::const_iterator pos (input.begin());
        const std::string::const_iterator end (input.end());
        std::size_t k (0);

        while (pos != end)
          {
            op_stack.push (token::eof);

            token::tokenizer token (k, pos, end);

            do
              {
                ++token;

                switch (*token)
                  {
                  case token::val:
                    nd_stack.push_back (token());
                    break;
                  case token::ref:
                    nd_stack.push_back (refnode(token.get_ref()));
                    break;
                  case token::define:
                    if (  nd_stack.empty()
                       || (! boost::apply_visitor
                             (node::visitor::is_ref (), nd_stack.back())
                          )
                       )
                      throw exception::parse::exception
                        ( "left hand of "
                        + fhg::util::show(*token)
                        + " must be reference name"
                        , k
                        );
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

    public:
      parser (const std::string & input, eval::context & context)
        : op_stack ()
        , nd_stack ()
      {
        parse (input, boost::bind ( eval::refnode_value
                                  , boost::ref(context)
                                  , _1
                                  )
              );
      }

      parser (const std::string & input)
        : op_stack ()
        , nd_stack ()
      {
        parse (input, eval::refnode_name);
      }

      parser (const nd_stack_t & seq)
        : op_stack ()
        , nd_stack (seq)
      {}

      // the parsed expressions in the correct order
      bool empty (void) const { return nd_stack.empty(); }
      void pop_front (void) { nd_stack.pop_front(); }
      const nd_t & front (void) const { return nd_stack.front(); }

      void add (parser & other)
      {
        while (!other.empty())
          {
            nd_stack.push_back (other.front()); other.pop_front();
          }
      }

      // eval the first entry in the stack
      value::type eval_front (eval::context & context) const
      {
        return boost::apply_visitor (eval::visitor::eval (context), front());
      }

      bool eval_front_bool (eval::context & context) const
      {
        return value::function::is_true(eval_front (context));
      }

      // get the already evaluated value, throws if entry is not an value
      const value::type & get_front () const
      {
        return node::get (front());
      }

      bool get_front_bool () const
      {
        return value::function::is_true(get_front ());
      }

      // evaluate the whole stack in order, return the last value
      value::type eval_all (eval::context & context) const
      {
        value::type v;

        for (nd_const_it_t it (begin()); it != end(); ++it)
          v = boost::apply_visitor (eval::visitor::eval (context), *it);

        return v;
      }

      bool eval_all_bool (eval::context & context) const
      {
        const value::type v (eval_all (context));

        return value::function::is_true(v);
      }

      void rename ( const key_vec_t::value_type & from
                  , const key_vec_t::value_type & to
                  )
      {
        for (nd_it_t it (begin()); it != end(); ++it)
          {
            node::rename (*it, from ,to);
          }
      }

      std::string string (void) const
      {
        std::ostringstream s;

        for (nd_const_it_t it (begin()); it != end(); ++it)
          {
            s << *it << ";";
          }

        return s.str();
      }

      friend std::ostream & operator << (std::ostream &, const parser &);
    };

    inline std::ostream & operator << (std::ostream & s, const parser & p)
    {
      for (parser::nd_const_it_t it (p.begin()); it != p.end(); ++it)
        s << *it << std::endl;
      return s << std::endl;
    }
  }
}

#endif
