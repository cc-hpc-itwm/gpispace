// bernd.loerwald@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_SIMPLIFY_EXPRESSION_LIST_HPP
#define _EXPR_PARSE_SIMPLIFY_EXPRESSION_LIST_HPP 1

#include <we/expr/parse/parser.hpp>
#include <boost/noncopyable.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      class expression_list : boost::noncopyable
      {
      public:
        typedef parser::nd_stack_t nodes_type;

      private:
        nodes_type _nodes;

      public:
        explicit expression_list (const parser & p)
        : _nodes (p.begin(), p.end())
        {}

        const nodes_type & nodes() const
        {
          return _nodes;
        }

        nodes_type::iterator begin () { return _nodes.begin(); }
        nodes_type::iterator end () { return _nodes.end(); }
        nodes_type::reverse_iterator rbegin () { return _nodes.rbegin(); }
        nodes_type::reverse_iterator rend () { return _nodes.rend(); }
        nodes_type::const_iterator begin () const { return _nodes.begin(); }
        nodes_type::const_iterator end () const { return _nodes.end(); }

        nodes_type::iterator erase (const nodes_type::iterator & it)
        {
          return _nodes.erase (it);
        }
        void insert (const nodes_type::iterator & it, nodes_type::value_type & value)
        {
          _nodes.insert (it, value);
        }
      };
    }
  }
}

#endif
