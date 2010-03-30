// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_TOKENIZER_HPP
#define _EXPR_TOKEN_TOKENIZER_HPP

#include <we/expr/token/prop.hpp>
#include <we/expr/token/type.hpp>

#include <we/expr/exception.hpp>

#include <we/util/show.hpp>

#include <string>
#include <iostream>

#include <math.h>

namespace expr
{
  namespace token
  {
    template<typename Key, typename Value, Key READ (const std::string &)>
    struct tokenizer
    {
    private:
      unsigned int & k;
      std::string::const_iterator & pos;
      const std::string::const_iterator & end;

      token::type token;
      Value tokval;
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
            throw expected ("'" + std::string (what_pos, what_end) + "'", k);
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

      void skip_comment (const unsigned int open)
      {
        while (pos != end)
          switch (*pos)
            {
            case '/':
              eat();
              if (pos != end && *pos == '*')
                {
                  eat(); skip_comment (k);
                }
              break;
            case '*':
              eat();
              if (pos != end && *pos == '/')
                {
                  eat(); return;
                }
              break;
            default: eat(); break;
            }

        throw unterminated_comment (open-2, k);
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
              if (is_eof())
                token = com;
              else
                switch (*pos)
                  {
                  case 'o': eat(); require("s"); unary (_cos, "cos"); break;
                  case 'e': eat(); require("il"); unary (_ceil, "ceil"); break;
                  default: token = com; break;
                  }
              break;
            case 'e':
              eat();
              if (is_eof())
                set_E();
              else
                switch (*pos)
                  {
                  case 'l': eat(); require("se"); token = _else; break;
                  case 'n': eat(); require("dif"); token = _endif; break;
                  default: set_E(); break;
                  }
              break;
            case 'f':
              eat();
              if (is_eof())
                token = fac;
              else
                switch (*pos)
                {
                case 'l': eat(); require("oor"); unary (_floor, "floor"); break;
                default: token = fac; break;
                }
              break;
            case 'i': eat(); require ("f"); token = _if; break;
            case 'l': eat(); require ("og"); unary (_log, "log"); break;
            case 'm':
              eat();
              if (is_eof())
                throw expected ("'in' or 'ax'", k);
              else
                switch (*pos)
                  {
                  case 'i': eat(); require ("n"); token = min; break;
                  case 'a': eat(); require ("x"); token = max; break;
                  default: throw expected ("'in' or 'ax'", k);
                  }
              break;
            case 'p': eat(); require("i"); set_PI(); break;
            case 'r': eat(); require("ound"); token = _round; break;
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
            case 't': eat(); require("hen"); token = _then; break;
            case '|': eat(); token = _or; break;
            case '&': eat(); token = _and; break;
            case '<': eat(); cmp (lt, le); break;
            case '>': eat(); cmp (gt, ge); break;
            case '!':
              eat();
              if (is_eof())
                throw expected("'=' or <expression>", k);
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
                throw expected("'='", k);
              else
                switch (*pos)
                  {
                  case '=': eat(); token = eq; break;
                  default: throw expected ("'='", k);
                  }
              break;
            case ':': eat(); require("="); token = define; break;
            case '+': eat(); token = add; break;
            case '-':
              eat();
              if (next_can_be_unary (token))
                token = neg;
              else
                token = sub;
              break;
            case '*': eat(); token = mul; break;

            case '/':
              eat();
              if (is_eof())
                token = div;
              else
                switch (*pos)
                  {
                  case '*': eat(); skip_comment(k); get(); break;
                  default: token = div; break;
                  }
              break;
            case '%': eat(); token = mod; break;
            case '^': eat(); token = _pow; break;
            case ',': eat(); token = sep; break;
            case '(': eat(); token = lpr; break;
            case ')': eat(); token = rpr; break;
            case '$':
              eat();
              token = ref;
              if (is_eof())
                throw expected ("'{'", k);
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
                    _ref = READ(_refname);
                    break;
                  default: throw expected ("'{'", k);
                  }
              break;
            default:
              if (!isdigit(*pos) && *pos != '.')
                throw expected ("<floating point value>", k);
              token = val;
              tokval = 0;
              while (pos != end && isdigit(*pos))
                {
                  tokval *= 10;
                  tokval += *pos - '0';
                  eat();
                }
              if (pos != end && *pos == '.')
                {
                  eat();
                  Value e (10);
                  while (pos != end && isdigit(*pos))
                    {
                      tokval += (*pos - '0') / e;
                      e *= 10;
                      eat();
                    }
                }
              if (pos != end && *pos == 'e')
                {
                  eat();
                  bool sig_neg (false);
                  if (pos != end && *pos == '-')
                    {
                      eat();
                      sig_neg = true;
                    }
                  else if (pos != end && *pos == '+')
                    eat();

                  if (pos == end || !isdigit(*pos))
                    throw expected ("<exponent>", k);

                  unsigned int e (0);

                  while (pos != end && isdigit(*pos))
                    {
                      e *= 10;
                      e += *pos - '0';
                      eat();
                    }

                  if (sig_neg)
                    tokval /= pow (10, e);
                  else
                    tokval *= pow (10, e);
                }
              break;
            }
      }

      template<typename K, typename V, K R (const V &)>
      friend std::ostream & operator << ( std::ostream &
                                        , const tokenizer<K, V, R> &
                                        );

    public:
      tokenizer ( unsigned int & _k
                , std::string::const_iterator & _pos
                , const std::string::const_iterator & _end
                ) 
        : k (_k), pos (_pos), end (_end), token (eof) {}
      const Value & operator () (void) const { return tokval; }
      const token::type & operator * (void) const { return token; }
      void operator ++ (void) { get(); }
      unsigned int eaten (void) const { return k; }
      const Key & get_ref (void) const { return _ref; }
    };

    template<typename Key, typename Value, Key R (const Value &)>
    static std::ostream & operator << ( std::ostream & s
                                      , const tokenizer<Key, Value, R> & t
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
