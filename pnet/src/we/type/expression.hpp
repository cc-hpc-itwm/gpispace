// {rahn,petry}@itwm.fhg.de

#ifndef WE_TYPE_EXPRESSION_HPP
#define WE_TYPE_EXPRESSION_HPP

#include <we/type/expression.fwd.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/expr/parse/util/get_names.hpp>
#include <we/expr/parse/simplify/simplify.hpp>

#include <boost/serialization/utility.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/nvp.hpp>

#include <string>
#include <iosfwd>

namespace we
{
  namespace type
  {
    struct expression_t
    {
      typedef expr::parse::parser ast_t;

      expression_t ();
      expression_t (const std::string& expr);

      // should correspond!
      expression_t (const std::string& expr, const ast_t& ast);

      const std::string& expression() const;
      const ast_t& ast() const;
      bool is_empty() const;

      bool simplify (const expr::parse::util::name_set_t& needed_bindings);

      void rename (const std::string& from, const std::string& to);

      void add (const expression_t&);

    private:
      std::string _expr;
      ast_t _ast;

      friend class boost::serialization::access;
      template <typename Archive>
      void save (Archive& ar, const unsigned int) const
      {
	ar << boost::serialization::make_nvp ("expr", _expr);
      }
      template <typename Archive>
      void load (Archive& ar, const unsigned int)
      {
	std::string tmp;
	ar >> boost::serialization::make_nvp ("expr", tmp);
	_ast = ast_t (tmp);
	_expr = tmp;
      }
      BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    std::ostream& operator<< (std::ostream&, const expression_t&);
  }
}

#endif
