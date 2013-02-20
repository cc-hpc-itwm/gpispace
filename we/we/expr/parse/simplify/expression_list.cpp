// bernd.loerwald@itwm.fraunhofer.de

#include <we/expr/parse/simplify/expression_list.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      expression_list::expression_list (const parser& p)
        : _nodes (p.begin(), p.end())
      { }

      const expression_list::nodes_type& expression_list::nodes() const
      {
        return _nodes;
      }

      expression_list::nodes_type::iterator expression_list::begin()
      {
        return _nodes.begin();
      }
      expression_list::nodes_type::iterator expression_list::end()
      {
        return _nodes.end();
      }

      expression_list::nodes_type::reverse_iterator expression_list::rbegin()
      {
        return _nodes.rbegin();
      }
      expression_list::nodes_type::reverse_iterator expression_list::rend()
      {
        return _nodes.rend();
      }

      expression_list::nodes_type::const_iterator expression_list::begin() const
      {
        return _nodes.begin();
      }
      expression_list::nodes_type::const_iterator expression_list::end() const
      {
        return _nodes.end();
      }

      expression_list::nodes_type::iterator expression_list::erase
        (const nodes_type::iterator& it)
      {
        return _nodes.erase (it);
      }
      void expression_list::insert
        (const nodes_type::iterator& it, nodes_type::value_type& value)
      {
        _nodes.insert (it, value);
      }
    }
  }
}
