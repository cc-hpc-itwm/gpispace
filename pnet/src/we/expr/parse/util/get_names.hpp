// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_UTIL_GET_NAMES_HPP
#define _EXPR_PARSE_UTIL_GET_NAMES_HPP 1

#include <boost/unordered_set.hpp>

#include <we/expr/parse/node.hpp>
#include <we/expr/parse/parser.hpp>

namespace expr
{
  namespace parse
  {
    namespace util
    {
      typedef boost::unordered_set<node::key_vec_t> name_set_t;

      name_set_t get_names (const node::type&);
      name_set_t get_names (const parser&);

      std::string write_key_vec (const name_set_t::value_type&);
    }

    std::ostream& operator<< (std::ostream&, util::name_set_t::value_type);
  }
}

#endif
