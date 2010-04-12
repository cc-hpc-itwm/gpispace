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

    const std::string & expression () const { return expr_; }
    const ast_t & ast () const { return ast_; }

    void expression (const std::string & expr)
    {
      ast_ = ast_t (expr);
      expr_ = expr;
    }

  private:
    std::string expr_;
    ast_t ast_;
  };

  inline std::ostream & operator << (std::ostream & os, const expression_t & e)
  {
    return os << e.expression();
  }
}}

namespace boost { namespace serialization {
  template<class Archive>
  inline void save
  ( Archive & ar
  , const we::type::expression_t & e
  , const unsigned int /* file_version */
  )
  {
    ar << e.expression();
  }

  template<class Archive, class Type, class Key, class Compare, class Allocator>
  inline void load
  ( Archive & ar
  , const we::type::expression_t & e
  , const unsigned int /* file_version */
  )
  {
    std::string tmp;
    ar >> tmp;
    e.expression (tmp);
  }

  // split non-intrusive serialization function member into separate
  // non intrusive save/load member functions
  template<class Archive, class Type, class Key, class Compare, class Allocator>
  inline void serialize
  ( Archive & ar
  , const we::type::expression_t & e
  , const unsigned int file_version
  )
  {
    boost::serialization::split_free(ar, e, file_version);
  }
}}

#endif
