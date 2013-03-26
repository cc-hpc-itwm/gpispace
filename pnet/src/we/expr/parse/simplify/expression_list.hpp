// bernd.loerwald@itwm.fraunhofer.de

#ifndef WE_EXPR_PARSE_SIMPLIFY_EXPRESSION_LIST_HPP
#define WE_EXPR_PARSE_SIMPLIFY_EXPRESSION_LIST_HPP

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
        explicit expression_list (const parser&);

        const nodes_type& nodes() const;

        nodes_type::iterator begin();
        nodes_type::iterator end();
        nodes_type::reverse_iterator rbegin();
        nodes_type::reverse_iterator rend();
        nodes_type::const_iterator begin() const;
        nodes_type::const_iterator end() const;

        nodes_type::iterator erase (const nodes_type::iterator&);
        void insert (const nodes_type::iterator&, nodes_type::value_type&);
      };
    }
  }
}

#endif
