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

#include <we/type/expression.fwd.hpp>

#include <string>
#include <we/expr/parse/parser.hpp>
#include <we/expr/parse/util/get_names.hpp>
#include <we/expr/parse/simplify/simplify.hpp>

#include <boost/serialization/utility.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/nvp.hpp>

namespace we { namespace type {
  struct expression_t
  {
    typedef expr::parse::parser ast_t;

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

    bool simplify (const expr::parse::util::name_set_t & needed_bindings)
    {
      ast_ = expr::parse::parser (expr::parse::simplify::simplification_pass
        (ast_, needed_bindings));

      bool modified (ast_.string() != expr_);
      if (modified)
      {
        expr_ = ast_.string();
      }
      return modified;
    }

    void rename ( const ast_t::key_vec_t::value_type & from
                , const ast_t::key_vec_t::value_type & to
                )
    {
      ast_.rename (from, to);
      expr_ = ast_.string();
    }

    void add (const expression_t & other)
    {
      ast_.add (other.ast());
      expr_ = ast_.string();
    }

  private:
    std::string expr_;
    ast_t ast_;

    friend class boost::serialization::access;
    template <typename Archive>
    void save (Archive & ar, const unsigned int) const
    {
      ar << boost::serialization::make_nvp("expr", expr_);
    }

    template <typename Archive>
    void load (Archive & ar, const unsigned int)
    {
      std::string tmp;
      ar >> boost::serialization::make_nvp("expr", tmp);
      ast_ = ast_t (tmp);
      expr_ = tmp;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()
  };

  inline std::ostream & operator << (std::ostream & os, const expression_t & e)
  {
    return os << e.expression();
  }
}}

#endif
