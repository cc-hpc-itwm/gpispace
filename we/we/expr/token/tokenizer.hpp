// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_TOKENIZER_HPP
#define _EXPR_TOKEN_TOKENIZER_HPP

#include <we/expr/token/type.hpp>

#include <we/type/value.hpp>
#include <we/type/value/container.hpp>

#include <fhg/util/parse/position.hpp>

#include <string>

namespace expr
{
  namespace token
  {
    typedef value::container::key_vec_t key_vec_t;

    struct tokenizer
    {
    private:
      fhg::util::parse::position pos;

      token::type token;
      value::type tokval;
      key_vec_t _ref;

      inline void set_E();
      inline void set_PI();
      inline bool is_eof();
      inline void require (const std::string&);
      inline void cmp (const token::type&, const token::type&);
      inline void unary (const token::type&, const std::string&);
      void skip_comment (const unsigned int);
      void get();

    public:
      tokenizer ( std::size_t&
                , std::string::const_iterator&
                , const std::string::const_iterator&
                );
      const value::type& operator()() const;
      const token::type& operator*() const;
      void operator++();
      const key_vec_t& get_ref() const;
    };
  }
}

#endif
