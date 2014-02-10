// bernd.loerwald@itwm.fraunhofer.de

#ifndef WE_EXPR_PARSE_SIMPLIFY_SIMPLIFY_HPP
#define WE_EXPR_PARSE_SIMPLIFY_SIMPLIFY_HPP

#include <we/expr/parse/parser.hpp>

#include <boost/unordered_set.hpp>

#include <list>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      typedef boost::unordered_set<std::list<std::string> > key_set_type;

      //! \todo also return the modified flag
      parser simplification_pass
        (const parser&, const key_set_type& needed_bindings);
    }
  }
}

#endif
