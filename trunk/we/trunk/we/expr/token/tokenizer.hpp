// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_TOKENIZER_HPP
#define _EXPR_TOKEN_TOKENIZER_HPP

#include <we/expr/token/prop.hpp>
#include <we/expr/token/type.hpp>

#include <we/expr/exception.hpp>

#include <string>
#include <iostream>

namespace expr
{
  namespace token
  {
    template<typename T>
    struct tokenizer
    {
    private:
      unsigned int & k;
      std::string::const_iterator & pos;
      const std::string::const_iterator & end;

      token::type token;
      T tokval;
      std::string _refname;

      inline void eat (void) { ++k; ++pos; }

      inline bool is_eof (void)
      {
        if (pos == end)
          return true;

        if (*pos == ';')
          {
            eat(); return true;
          }

        return false;
      }

      inline void require (const std::string & what)
      {
        std::string::const_iterator what_pos (what.begin());
        const std::string::const_iterator what_end (what.end());

        while (what_pos != what_end)
          if (is_eof() || *pos != *what_pos)
            throw expected (std::string (what_pos, what_end), k);
          else
            {
              eat(); ++what_pos;
            }
      }

      inline void cmp (const token::type & t, const token::type & e)
      {
        if (is_eof())
          token = t;
        else
          switch (*pos)
            {
            case '=': eat(); token = e; break;
            default: token = t; break;
            }
      }

      inline void unary (const token::type & t, const std::string descr)
      {
        if (next_can_be_unary (token))
          token = t;
        else
          throw misplaced (descr, k);
      }

      void get (void)
      {
        while (pos != end && isspace(*pos))
          eat();

        if (is_eof())
          token = eof;
        else
          switch (*pos)
            {
            case 'a': eat(); require ("bs"); unary (abs, "abs"); break;
            case 'c':
              eat();
              if (is_eof() || *pos != 'o')
                token = com;
              else
                {
                  eat(); require ("s"); unary (_cos, "cos");
                }
              break;
            case 'e': eat(); token = val; tokval = 2.7182818284590452354; break;
            case 'f': eat(); token = fac; break;
            case 'l': eat(); require ("og"); unary (_log, "log"); break;
            case 'm':
              eat();
              if (is_eof())
                throw expected ("in or ax", k);
              else
                switch (*pos)
                  {
                  case 'i': eat(); require ("n"); token = min; break;
                  case 'a': eat(); require ("x"); token = max; break;
                  default: throw expected ("in or ax", k);
                  }
              break;
            case 'p':
              eat(); require("i"); token = val; tokval = 3.14159265358979323846;
              break;
            case 's':
              eat();
              if (is_eof())
                throw expected ("'in' or 'qrt'", k);
              else
                switch (*pos)
                  {
                  case 'i': eat(); require ("n"); unary (_sin, "sin"); break;
                  case 'q': eat(); require ("rt"); unary (_sqrt, "sqrt"); break;
                  default: throw expected ("'in' or 'qrt'", k);
                  }
              break;
            case '|': eat(); token = _or; break;
            case '&': eat(); token = _and; break;
            case '<': eat(); cmp (lt, le); break;
            case '>': eat(); cmp (gt, ge); break;
            case '!':
              eat();
              if (is_eof())
                throw expected("= or expression", k);
              else
                switch (*pos)
                  {
                  case '=': eat(); token = ne; break;
                  default: unary (_not, "negation"); break;
                  }
              break;
            case '=':
              eat();
              if (is_eof())
                throw expected("=", k);
              else
                switch (*pos)
                  {
                  case '=': eat(); token = eq; break;
                  default: throw expected ("=", k);
                  }
              break;
            case '+': eat(); token = add; break;
            case '-':
              eat();
              if (next_can_be_unary (token))
                token = neg;
              else
                token = sub;
              break;
            case '*': eat(); token = mul; break;
            case '/': eat(); token = div; break;
            case '%': eat(); token = mod; break;
            case '^': eat(); token = pow; break;
            case ',': eat(); token = sep; break;
            case '(': eat(); token = lpr; break;
            case ')': eat(); token = rpr; break;
            case '$':
              eat();
              token = ref;
              if (is_eof())
                throw expected ("{", k);
              else
                switch (*pos)
                  {
                  case '{':
                    eat();
                    _refname.clear();
                    while (pos != end && *pos != '}')
                      {
                        _refname.push_back (*pos);
                        eat();
                      }
                    require ("}");
                    break;
                  default: throw expected ("{", k);
                  }
              break;
            default:
              if (!isdigit(*pos) && *pos != '.')
                throw expected ("expression", k);
              token = val;
              tokval = 0;
              while (isdigit(*pos))
                {
                  tokval *= 10;
                  tokval += *pos - '0';
                  eat();
                }
              if (*pos == '.')
                {
                  eat();
                  T e (10);
                  while (isdigit(*pos))
                    {
                      tokval += (*pos - '0') / e;
                      e *= 10;
                      eat();
                    }
                }
              if (*pos == 'e')
                {
                  eat();
                  bool sig_neg (false);
                  if (*pos == '-')
                    {
                      eat();
                      sig_neg = true;
                    }
                  else if (*pos == '+')
                    eat();

                  if (!isdigit(*pos))
                    throw expected ("digit", k);

                  unsigned int e (0);

                  while (isdigit(*pos))
                    {
                      e *= 10;
                      e += *pos - '0';
                      eat();
                    }

                  while (e-->0)
                    if (sig_neg)
                      tokval /= 10;
                    else
                      tokval *= 10;
                }
              break;
            }
      }

      template<typename U>
      friend std::ostream & operator << (std::ostream &, const tokenizer<U> &);

    public:
      tokenizer ( unsigned int & _k
                , std::string::const_iterator & _pos
                , const std::string::const_iterator & _end
                ) 
        : k (_k), pos (_pos), end (_end), token (eof) {}
      const T & operator () (void) const { return tokval; }
      const token::type & operator * (void) const { return token; }
      void operator ++ (void) { get(); }
      unsigned int eaten (void) const { return k; }
      const std::string & refname (void) const { return _refname; }
    };

    template<typename T>
    static std::ostream & operator << (std::ostream & s, const tokenizer<T> & t)
    {
      switch (*t)
        {
        case val: return s << t();
        case ref: return s << "${" << t.refname() << "}";
        default: return s << *t;
        }
    }
  }
}

#endif
