/*
 * =====================================================================================
 *
 *       Filename:  expression.hpp
 *
 *    Description:  expression descriptor
 *
 *        Version:  1.0
 *        Created:  04/13/2010 12:31:25 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_TYPE_EXPRESSION_HPP
#define WE_TYPE_EXPRESSION_HPP

#include <string>
#include <we/expr/parse/parser.hpp>

#include <boost/serialization/utility.hpp>
#include <boost/serialization/split_free.hpp>

namespace we { namespace type {
  struct expression_t
  {
    typedef expr::parse::parser<std::string> ast_t;

    expression_t ()
      : expr_("")
      , ast_("")
    {}

    expression_t (const std::string & expr)
      : expr_(expr)
      , ast_(expr)
    {}

    // should correspond!
    expression_t (const std::string & expr, const ast_t & ast)
      : expr_(expr)
      , ast_(ast)
    {}

    const std::string & expression () const { return expr_; }
    const ast_t & ast () const { return ast_; }
    bool is_empty () const { return expr_ == ""; }

    void expression (const std::string & expr)
    {
      ast_ = ast_t (expr);
      expr_ = expr;
    }

  private:
    std::string expr_;
    ast_t ast_;

    friend class boost::serialization::access;
    template <typename Archive>
    void save (Archive & ar, const unsigned int) const
    {
      ar << expr_;
    }

    template <typename Archive>
    void load (Archive & ar, const unsigned int)
    {
      std::string tmp;
      ar >> tmp;
      expression (tmp);
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()
  };

  inline std::ostream & operator << (std::ostream & os, const expression_t & e)
  {
    return os << e.expression();
  }
}}

#endif
