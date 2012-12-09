// mirko.rahn@itwm.fhg.de

#include <we/type/expression.hpp>

#include <ostream>

namespace we
{
  namespace type
  {
    expression_t::expression_t ()
      : _expr ("")
      , _ast ("")
    {}

    expression_t::expression_t (const std::string& expr)
      : _expr (expr)
      , _ast (expr)
    {}

    // should correspond!
    expression_t::expression_t (const std::string& expr, const ast_t& ast)
      : _expr (expr)
      , _ast (ast)
    {}

    const std::string& expression_t::expression() const
    {
      return _expr; 
    }

    const expression_t::ast_t& expression_t::ast() const
    {
      return _ast;
    }

    bool expression_t::is_empty() const
    {
      return _expr.empty();
    }

    bool expression_t::simplify 
    (const expr::parse::util::name_set_t& needed_bindings)
    {
      _ast = expr::parse::parser (expr::parse::simplify::simplification_pass
        (_ast, needed_bindings));

      const bool modified (_ast.string() != _expr);

      if (modified)
      {
        _expr = _ast.string();
      }

      return modified;
    }

    void expression_t::rename ( const ast_t::key_vec_t::value_type& from
			      , const ast_t::key_vec_t::value_type& to
			      )
    {
      _ast.rename (from, to);
      _expr = _ast.string();
    }

    void expression_t::add (const expression_t& other)
    {
      _ast.add (other.ast());
      _expr = _ast.string();
    }

    std::ostream& operator<< (std::ostream& os, const expression_t& e)
    {
      return os << e.expression();
    }
  }
}
