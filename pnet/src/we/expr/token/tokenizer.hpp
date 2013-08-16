// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_TOKENIZER_HPP
#define _EXPR_TOKEN_TOKENIZER_HPP

#include <we/expr/token/type.hpp>

#include <we/type/value.hpp>

#include <fhg/util/parse/position.hpp>

#include <string>
#include <list>

namespace expr
{
  namespace token
  {
    struct tokenizer
    {
    public:
      tokenizer (fhg::util::parse::position&);
      const pnet::type::value::value_type& value() const;
      const token::type& token() const;
      void operator++();
      const std::list<std::string>& get_ref() const;

    public:
      void set_token (const token::type&);
      void set_value (const pnet::type::value::value_type&);
      void unary (const token::type&, const std::string&);
      void cmp (const token::type&, const token::type&);
      void negsub();
      void mulpow();
      void divcomment();
      void identifier();
      void notne();
      bool is_eof();
      fhg::util::parse::position& pos();

    private:
      fhg::util::parse::position& _pos;
      token::type _token;
      pnet::type::value::value_type _tokval;
      std::list<std::string> _ref;

      void skip_comment (const std::size_t);
    };
  }
}

#endif
