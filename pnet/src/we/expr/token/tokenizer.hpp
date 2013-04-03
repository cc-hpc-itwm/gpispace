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
    typedef std::list<std::string> key_vec_t;

    struct tokenizer
    {
    public:
      void set_token (const token::type&);
      void set_value (const value::type&);
      void unary (const token::type&, const std::string&);
      void cmp (const token::type&, const token::type&);
      void negsub();
      void mulpow();
      void divcomment();
      void identifier();
      void notne();
      fhg::util::parse::position& pos;

    private:
      token::type token;
      value::type tokval;
      std::list<std::string> _ref;

      bool is_eof();
      void skip_comment (const unsigned int);
      void get();

    public:
      tokenizer (fhg::util::parse::position& pos);
      const value::type& operator()() const;
      const token::type& operator*() const;
      void operator++();
      const std::list<std::string>& get_ref() const;
    };
  }
}

#endif
