// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_TOKENIZER_HPP
#define _EXPR_TOKEN_TOKENIZER_HPP

#include <we/expr/token/prop.hpp>
#include <we/expr/token/type.hpp>

#include <we/expr/parse/position.hpp>

#include <we/expr/exception.hpp>

#include <we/expr/variant/variant.hpp>

#include <we/util/show.hpp>

#include <string>
#include <iostream>

#include <math.h>

namespace expr
{
  namespace token
  {
    template<typename Key, typename READER>
    struct tokenizer
    {
    private:
      parse::position pos;

      token::type token;
      variant::type tokval;
      std::string _refname;
      Key _ref;

      inline void set_E (void)
      {
        token = val;
        tokval = 2.7182818284590452354;
      }

      inline void set_PI (void)
      {
        token = val;
        tokval = 3.14159265358979323846;
      }

      inline bool is_eof (void)
      {
        if (pos.end())
          return true;

        if (*pos == ';')
          {
            ++pos; return true;
          }

        return false;
      }

      inline void require (const std::string & what)
      {
        std::string::const_iterator what_pos (what.begin());
        const std::string::const_iterator what_end (what.end());

        while (what_pos != what_end)
          if (is_eof() || *pos != *what_pos)
            throw exception::parse::expected
              ("'" + std::string (what_pos, what_end) + "'", pos());
          else
            {
              ++pos; ++what_pos;
            }
      }

      inline void cmp (const token::type & t, const token::type & e)
      {
        if (is_eof())
          token = t;
        else
          switch (*pos)
            {
            case '=': ++pos; token = e; break;
            default: token = t; break;
            }
      }

      inline void unary (const token::type & t, const std::string descr)
      {
        if (next_can_be_unary (token))
          token = t;
        else
          throw exception::parse::misplaced (descr, pos());
      }

      void skip_comment (const unsigned int open)
      {
        while (!pos.end())
          switch (*pos)
            {
            case '/':
              ++pos;
              if (!pos.end() && *pos == '*')
                {
                  ++pos; skip_comment (pos());
                }
              break;
            case '*':
              ++pos;
              if (!pos.end() && *pos == '/')
                {
                  ++pos; return;
                }
              break;
            default: ++pos; break;
            }

        throw exception::parse::unterminated ("comment", open-2, pos());
      }

      void get (void)
      {
        while (!pos.end() && isspace(*pos))
          ++pos;

        if (is_eof())
          token = eof;
        else
          switch (*pos)
            {
            case 'a': ++pos; require ("bs"); unary (abs, "abs"); break;
            case 'c':
              ++pos;
              if (is_eof())
                throw exception::parse::expected ("'os' or 'eil'", pos());
              else
                switch (*pos)
                  {
                  case 'o': ++pos; require("s"); unary (_cos, "cos"); break;
                  case 'e': ++pos; require("il"); unary (_ceil, "ceil"); break;
                  default: throw exception::parse::expected 
                      ("'os' or 'eil'", pos());
                  }
              break;
            case 'd':
              ++pos;
              if (pos.end())
                throw exception::parse::expected ("'iv' or 'ouble'", pos());
              else
                switch (*pos)
                  {
                  case 'i': ++pos; require ("v"); token = divint; break;
                  case 'o': ++pos; require ("uble"); token = _todouble; break;
                  default: throw exception::parse::expected
                      ("'iv' or 'ouble'", pos());
                  }
              break;
            case 'e':
              ++pos;
              if (is_eof())
                set_E();
              else
                switch (*pos)
                  {
                  case 'l': ++pos; require("se"); token = _else; break;
                  case 'n': ++pos; require("dif"); token = _endif; break;
                  default: set_E(); break;
                  }
              break;
            case 'f':
              ++pos;
              if (pos.end())
                throw exception::parse::expected ("'loor' or 'alse'", pos());
              else
                switch (*pos)
                  {
                  case 'l':
                    ++pos; require ("oor"); unary (_floor, "floor"); break;
                  case 'a':
                    ++pos; require ("lse"); token = val; tokval = false; break;
                  default:
                    throw exception::parse::expected ("'loor' or 'alse'", pos());
                  }
              break;
            case 'i':
              ++pos;
              if (pos.end())
                throw exception::parse::expected ("'f' of 'nt'", pos());
              else
                switch (*pos)
                  {
                  case 'f': ++pos; token = _if; break;
                  case 'n': ++pos; require("t"); token = _toint; break;
                  default: throw exception::parse::expected ("'f' of 'nt'", pos());
                  }
              break;
            case 'l': ++pos; require ("og"); unary (_log, "log"); break;
            case 'm':
              ++pos;
              if (is_eof())
                throw exception::parse::expected
                  ("'in' or 'ax' or 'od'", pos());
              else
                switch (*pos)
                  {
                  case 'i': ++pos; require ("n"); token = min; break;
                  case 'a': ++pos; require ("x"); token = max; break;
                  case 'o': ++pos; require ("d"); token = modint; break;
                  default: throw exception::parse::expected
                      ("'in' or 'ax' od 'od'", pos());
                  }
              break;
            case 'p': ++pos; require("i"); set_PI(); break;
            case 'r': ++pos; require("ound"); token = _round; break;
            case 's':
              ++pos;
              if (is_eof())
                throw exception::parse::expected ("'in' or 'qrt'", pos());
              else
                switch (*pos)
                  {
                  case 'i': ++pos; require ("n"); unary (_sin, "sin"); break;
                  case 'q': ++pos; require ("rt"); unary (_sqrt, "sqrt"); break;
                  default: throw exception::parse::expected
                      ("'in' or 'qrt'", pos());
                  }
              break;
            case 't':
              ++pos;
              if (pos.end())
                throw exception::parse::expected ("'hen' or 'rue'", pos());
              else
                switch (*pos)
                  {
                  case 'h': ++pos; require("en"); token = _then; break;
                  case 'r': ++pos; require("ue"); token = val; tokval = true; break;
                  default: throw exception::parse::expected ("'hen' or 'rue'", pos());
                  }
              break;
            case '|': ++pos; token = _or; break;
            case '&': ++pos; token = _and; break;
            case '<': ++pos; cmp (lt, le); break;
            case '>': ++pos; cmp (gt, ge); break;
            case '!':
              ++pos;
              if (is_eof())
                throw exception::parse::expected("'=' or <expression>", pos());
              else
                switch (*pos)
                  {
                  case '=': ++pos; token = ne; break;
                  default: unary (_not, "negation"); break;
                  }
              break;
            case '=':
              ++pos;
              if (is_eof())
                throw exception::parse::expected("'='", pos());
              else
                switch (*pos)
                  {
                  case '=': ++pos; token = eq; break;
                  default: throw exception::parse::expected ("'='", pos());
                  }
              break;
            case ':': ++pos; require("="); token = define; break;
            case '+': ++pos; token = add; break;
            case '-':
              ++pos;
              if (next_can_be_unary (token))
                token = neg;
              else
                token = sub;
              break;
            case '*':
              ++pos;
              if (!pos.end() && *pos == '*')
                {
                  token = _pow;
                  ++pos;
                }
              else
                token = mul;
              break;
            case '/':
              ++pos;
              if (is_eof())
                token = div;
              else
                switch (*pos)
                  {
                  case '*': ++pos; skip_comment(pos()); get(); break;
                  default: token = div; break;
                  }
              break;
            case '%': ++pos; token = mod; break;
            case '^': ++pos; token = _powint; break;
            case ',': ++pos; token = sep; break;
            case '(': ++pos; token = lpr; break;
            case ')': ++pos; token = rpr; break;
            case '$':
              ++pos;
              token = ref;
              if (is_eof())
                throw exception::parse::expected ("'{'", pos());
              else
                switch (*pos)
                  {
                  case '{':
                    ++pos;
                    _refname.clear();
                    while (!pos.end() && *pos != '}')
                      {
                        _refname.push_back (*pos);
                        ++pos;
                      }
                    require ("}");
                    _ref = READER::read(_refname);
                    break;
                  default: throw exception::parse::expected ("'{'", pos());
                  }
              break;
            default: token = val; variant::read (tokval, pos); break;
            }
      }

      template<typename K, typename R>
      friend std::ostream & operator << ( std::ostream &
                                        , const tokenizer<K, R> &
                                        );

    public:
      tokenizer ( unsigned int & _k
                , std::string::const_iterator & _pos
                , const std::string::const_iterator & _end
                ) 
        : pos (_k, _pos,_end), token (eof) {}
      const variant::type & operator () (void) const { return tokval; }
      const token::type & operator * (void) const { return token; }
      void operator ++ (void) { get(); }
      const Key & get_ref (void) const { return _ref; }
    };

    template<typename Key, typename R>
    static std::ostream & operator << ( std::ostream & s
                                      , const tokenizer<Key, R> & t
                                      )
    {
      switch (*t)
        {
        case val: return s << t();
        case ref: return s << "${" << show(t.ref()) << "}";
        default: return s << *t;
        }
    }
  }
}

#endif
