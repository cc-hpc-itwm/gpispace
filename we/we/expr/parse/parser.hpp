// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_PARSER_HPP
#define _EXPR_PARSE_PARSER_HPP

#include <we/expr/parse/node.hpp>

#include <we/expr/exception.hpp>
#include <we/expr/token/type.hpp>

#include <we/type/value.hpp>

#include <boost/function.hpp>

#include <string>
#include <stack>
#include <list>

#include <ostream>

namespace expr
{
  namespace eval
  {
    struct context;
  }

  namespace exception
  {
    namespace parse
    {
      class missing_operand : public exception
      {
      public:
        missing_operand (const std::size_t k, const std::string & what)
          : exception ("missing " + what + " operand", k) {}
        missing_operand (const std::size_t k)
          : exception ("missing operand", k) {}
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
      nd_stack_t tmp_stack;
      bool _constant_folding;

      nd_it_t begin () { return nd_stack.begin(); }
      nd_it_t end () { return nd_stack.end(); }
      const bool& constant_folding() { return _constant_folding; }

    public:
      nd_const_it_t begin () const { return nd_stack.begin(); }
      nd_const_it_t end () const { return nd_stack.end(); }

    private:
      void unary (const token::type & token, const std::size_t k);
      void binary (const token::type & token, const std::size_t k);
      void ternary (const token::type & token, const std::size_t k);
      void ite (const std::size_t k);
      void reduce (const std::size_t k);
      void
      parse ( const std::string& input
            , const boost::function<nd_t (const key_vec_t &)> & refnode
            );

    public:
      parser ( const std::string & input
             , eval::context & context
             , const bool& constant_folding = true
             );
      parser ( const std::string & input
             , const bool& constant_folding = true
             );
      parser ( const nd_stack_t & seq
             , const bool& constant_folding = true
             );

      // the parsed expressions in the correct order
      bool empty (void) const { return nd_stack.empty(); }
      void pop_front (void) { nd_stack.pop_front(); }
      const nd_t & front (void) const { return nd_stack.front(); }

      void add (const parser & other);

      // eval the first entry in the stack
      value::type eval_front (eval::context & context) const;
      bool eval_front_bool (eval::context & context) const;

      // get the already evaluated value, throws if entry is not an value
      const value::type & get_front () const;
      bool get_front_bool () const;

      // evaluate the whole stack in order, return the last value
      value::type eval_all (eval::context & context) const;
      bool eval_all_bool (eval::context & context) const;

      value::type eval_all() const;

      void rename ( const key_vec_t::value_type & from
                  , const key_vec_t::value_type & to
                  );

      std::string string (void) const;

      friend std::ostream & operator << (std::ostream &, const parser &);
    };

    std::ostream& operator << (std::ostream&, const parser&);
    std::string parse_result ( const std::string&
                             , const bool& constant_folding = false
                             );
  }
}

#endif
