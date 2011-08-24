// bernd.loerwald@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_SIMPLIFY_EXPRESSION_LIST_HPP
#define _EXPR_PARSE_SIMPLIFY_EXPRESSION_LIST_HPP 1

#include <we/expr/parse/parser.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      struct expression_list
      {
        typedef parser::nd_stack_t node_stack_t;
        typedef node_stack_t::value_type node_t;
        typedef node_stack_t::iterator node_stack_it_t;
        typedef node_stack_t::reverse_iterator node_stack_r_it_t;
        typedef node_stack_t::const_iterator node_stack_const_it_t;

      private:
        node_stack_t _node_stack;

      public:
        const node_stack_t& node_stack() const
        {
          return _node_stack;
        }

        explicit expression_list (const parser & p)
        : _node_stack (p.begin(), p.end())
        {}

        node_stack_it_t begin () { return _node_stack.begin(); }
        node_stack_it_t end () { return _node_stack.end(); }
        node_stack_r_it_t rbegin () { return _node_stack.rbegin(); }
        node_stack_r_it_t rend () { return _node_stack.rend(); }

        node_stack_const_it_t begin () const { return _node_stack.begin(); }
        node_stack_const_it_t end () const { return _node_stack.end(); }

        node_stack_it_t erase (const node_stack_it_t & it)
        {
          return _node_stack.erase (it);
        }
        void insert (const node_stack_it_t & it, node_t & value)
        {
          _node_stack.insert (it, value);
        }
      };
    }
  }
}

#endif
