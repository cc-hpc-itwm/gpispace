// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_TOKENIZER_HPP
#define _EXPR_TOKEN_TOKENIZER_HPP

#include <expr/exception.hpp>
#include <expr/token/type.hpp>
#include <expr/token/prop.hpp>

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
      std::string::const_iterator pos;
      const std::string::const_iterator end;
      token::type token;
      T tokval;
      std::string _refname;

      void get (void)
      {
        while (pos != end && isspace(*pos))
          ++pos;

        if (pos == end)
          token = eof;
        else
          switch (*pos)
            {
            case 'a':
              ++pos;
              if (pos == end || *pos != 'b')
                throw expected ("bs");
              else
                {
                  ++pos;
                  if (pos == end || *pos != 's')
                    throw expected ("s");
                  else
                    {
                      if (next_can_be_unary (token))
                        token = abs;
                      else
                        throw exception ("misplaced abs");
                      ++pos;
                    }
                }
              break;
            case '|': token = _or; ++pos; break;
            case '&': token = _and; ++pos; break;
            case '<':
              ++pos;
              if (pos == end)
                token = lt;
              else
                switch (*pos)
                  {
                  case '=': token = le; ++pos; break;
                  default: token = lt; break;
                  }
              break;
            case '>':
              ++pos;
              if (pos == end)
                token = gt;
              switch (*pos)
                {
                case '=': token = ge; ++pos; break;
                default: token = gt; break;
                }
              break;
            case '!':
              ++pos;
              if (pos == end)
                throw expected("= or expression");
              else
                switch (*pos)
                  {
                  case '=': token = ne; ++pos; break;
                  default:
                    if (next_can_be_unary (token))
                      token = _not;
                    else
                      throw exception ("misplaced negation");
                    break;
                  }
              break;
            case '=':
              ++pos;
              if (pos == end)
                throw expected("=");
              else
                switch (*pos)
                  {
                  case '=': token = eq; ++pos; break;
                  default: throw expected ("=");
                  }
              break;

            case '+': token = add; ++pos; break;
            case '-':
              if (next_can_be_unary (token))
                token = neg;
              else
                token = sub;
              ++pos;
              break;
            case '*': token = mul; ++pos; break;
            case '/': token = div; ++pos; break;
            case '%': token = mod; ++pos; break;
            case '^': token = pow; ++pos; break;
            case 'f': token = fac; ++pos; break;
            case 'c': token = com; ++pos; break;
            case 'm':
              ++pos;
              if (pos == end)
                throw expected ("in or ax");
              else
                switch (*pos)
                  {
                  case 'i':
                    ++pos;
                    if (pos == end)
                      throw expected ("n");
                    else
                      if (*pos == 'n')
                        {
                          token = min;
                          ++pos;
                        }
                      else
                        throw expected ("n");
                    break;
                  case 'a':
                    ++pos;
                    if (pos == end)
                      throw expected ("x");
                    else
                      if (*pos == 'x')
                        {
                          token = max;
                          ++pos;
                        }
                      else
                        throw expected ("x");
                    break;
                  default: throw expected ("in or ax");
                  }
              break;
            case ',': token = sep; ++pos; break;
            case '(': token = lpr; ++pos; break;
            case ')': token = rpr; ++pos; break;
            case '$':
              token = ref;
              ++pos;
              if (pos == end)
                throw expected ("{");
              else
                switch (*pos)
                  {
                  case '{':
                    ++pos;
                    _refname.clear();
                    while (pos != end && *pos != '}')
                      {
                        _refname.push_back (*pos);
                        ++pos;
                      }
                    if (*pos != '}')
                      throw expected ("}");
                    ++pos;
                    break;
                  default: throw expected ("{");
                  }
              break;
            default:
              if (!isdigit(*pos))
                throw expected ("digit");
              token = val;
              tokval = 0;
              while (isdigit(*pos))
                {
                  tokval *= 10;
                  tokval += *pos - '0';
                  ++pos;
                }
              break;
            }
      }

      template<typename U>
      friend std::ostream & operator << (std::ostream &, const tokenizer<U> &);

    public:
      tokenizer (const std::string & input) 
        : pos (input.begin()), end (input.end()), token (eof) {}
      const T & operator () (void) const { return tokval; }
      const token::type & operator * (void) const { return token; }
      void operator ++ (void) { get(); }
      std::string refname (void) const { return _refname; }
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
