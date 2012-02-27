/*
 * =====================================================================================
 *
 *       Filename:  expr.hpp
 *
 *    Description:  expression parser using Boost.Spirit
 *
 *        Version:  1.0
 *        Created:  02/01/2010 05:04:31 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_EXPR_HPP
#define WE_EXPR_HPP 1

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_placeholders.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/foreach.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace we { namespace expr {
  namespace fusion = boost::fusion;
  namespace phoenix = boost::phoenix;
  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;

  /*
   * expression examples:
   *	${i0} -> references the value of some input variable
   *	out = ${i0} -> assigns input i0 to output "out"
   *
   *	${i0} + ${i1} < 10 -> compares the sum of i0 and i1 to the number 10
   *	${i0} * ${i1} + 10
   *
   */

  enum OP_TYPE
  {
	OP_PLUS      // +
	  , OP_MINUS     // -
	  , OP_DIV       // /
	  , OP_MUL       // *
	  , OP_MOD       // %
	  , OP_LT        // <
	  , OP_LEQ       // <=
	  , OP_EQ        // ==
	  , OP_NEQ       // !=
	  , OP_GT        // >
	  , OP_GEQ       // >=
  };

  struct operators_ : qi::symbols<char, OP_TYPE>
  {
	operators_()
	{
	  add
		("+" , OP_PLUS  )
		("-" , OP_MINUS )
		("/" , OP_DIV   )
		("*" , OP_MUL   )
		("%" , OP_MOD   )
		("<" , OP_LT    )
		("<=", OP_LEQ   )
		("==", OP_EQ    )
		("!=", OP_NEQ   )
		(">" , OP_GT    )
		(">=", OP_GEQ   )
		;
	}
  } operators;

  typedef boost::variant<std::string, int, double> factor_t;

  //  struct expression_tree
  //  {
  //	std::string b;
  //  };
  typedef std::string expression_tree;

  //  template <typename Identifier = std::string>
  //  struct assignment_t
  //  {
  //	Identifier lval;
  //	statement_t <Identifier> expr;
  //  };

  template <typename Iterator>
	struct expression_grammar : qi::grammar<Iterator, expression_tree(), ascii::space_type>
  {
	expression_grammar() : expression_grammar::base_type(expr, "expression")
	{
//	  using qi::lit;
//	  using qi::lexeme;
//	  using ascii::char_;
//	  using ascii::string;
//
//	  using phoenix::at_c;
//	  using phoenix::push_back;
	}

	qi::rule<Iterator, expression_tree()> expr;
	qi::rule<Iterator, std::string(), ascii::space_type> identifier;
  };
}}

#endif
