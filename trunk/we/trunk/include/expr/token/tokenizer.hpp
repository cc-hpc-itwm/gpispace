// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_TOKENIZER_HPP
#define _EXPR_TOKEN_TOKENIZER_HPP

#include <expr/token/prop.hpp>
#include <expr/token/type.hpp>

#include <expr/exception.hpp>

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
      unsigned int k;
      std::string::const_iterator pos;
      const std::string::const_iterator end;
      token::type token;
      T tokval;
      std::string _refname;

      void eat (void) { ++k; ++pos; }

      void get (void)
      {
        while (pos != end && isspace(*pos))
          eat();;

        if (pos == end)
          token = eof;
        else
          switch (*pos)
            {
            case 'a':
              eat();;
              if (pos == end || *pos != 'b')
                throw expected ("bs", k);
              else
                {
                  eat();;
                  if (pos == end || *pos != 's')
                    throw expected ("s", k);
                  else
                    {
                      if (next_can_be_unary (token))
                        token = abs;
                      else
                        throw exception ("misplaced abs", k);
                      eat();;
                    }
                }
              break;
            case '|': token = _or; eat();; break;
            case '&': token = _and; eat();; break;
            case '<':
              eat();;
              if (pos == end)
                token = lt;
              else
                switch (*pos)
                  {
                  case '=': token = le; eat();; break;
                  default: token = lt; break;
                  }
              break;
            case '>':
              eat();;
              if (pos == end)
                token = gt;
              switch (*pos)
                {
                case '=': token = ge; eat();; break;
                default: token = gt; break;
                }
              break;
            case '!':
              eat();;
              if (pos == end)
                throw expected("= or expression", k);
              else
                switch (*pos)
                  {
                  case '=': token = ne; eat();; break;
                  default:
                    if (next_can_be_unary (token))
                      token = _not;
                    else
                      throw exception ("misplaced negation", k);
                    break;
                  }
              break;
            case '=':
              eat();;
              if (pos == end)
                throw expected("=", k);
              else
                switch (*pos)
                  {
                  case '=': token = eq; eat();; break;
                  default: throw expected ("=", k);
                  }
              break;

            case '+': token = add; eat();; break;
            case '-':
              if (next_can_be_unary (token))
                token = neg;
              else
                token = sub;
              eat();;
              break;
            case '*': token = mul; eat();; break;
            case '/': token = div; eat();; break;
            case '%': token = mod; eat();; break;
            case '^': token = pow; eat();; break;
            case 'f': token = fac; eat();; break;
            case 'c': token = com; eat();; break;
            case 'm':
              eat();;
              if (pos == end)
                throw expected ("in or ax", k);
              else
                switch (*pos)
                  {
                  case 'i':
                    eat();;
                    if (pos == end)
                      throw expected ("n", k);
                    else
                      if (*pos == 'n')
                        {
                          token = min;
                          eat();;
                        }
                      else
                        throw expected ("n", k);
                    break;
                  case 'a':
                    eat();;
                    if (pos == end)
                      throw expected ("x", k);
                    else
                      if (*pos == 'x')
                        {
                          token = max;
                          eat();;
                        }
                      else
                        throw expected ("x", k);
                    break;
                  default: throw expected ("in or ax", k);
                  }
              break;
            case ',': token = sep; eat();; break;
            case '(': token = lpr; eat();; break;
            case ')': token = rpr; eat();; break;
            case '$':
              token = ref;
              eat();;
              if (pos == end)
                throw expected ("{", k);
              else
                switch (*pos)
                  {
                  case '{':
                    eat();;
                    _refname.clear();
                    while (pos != end && *pos != '}')
                      {
                        _refname.push_back (*pos);
                        eat();;
                      }
                    if (*pos != '}')
                      throw expected ("}", k);
                    eat();;
                    break;
                  default: throw expected ("{", k);
                  }
              break;
            default:
              if (!isdigit(*pos))
                throw expected ("digit", k);
              token = val;
              tokval = 0;
              while (isdigit(*pos))
                {
                  tokval *= 10;
                  tokval += *pos - '0';
                  eat();;
                }
              break;
            }
      }

      template<typename U>
      friend std::ostream & operator << (std::ostream &, const tokenizer<U> &);

    public:
      tokenizer (const std::string & input) 
        : k (0), pos (input.begin()), end (input.end()), token (eof) {}
      const T & operator () (void) const { return tokval; }
      const token::type & operator * (void) const { return token; }
      void operator ++ (void) { get(); }
      unsigned int eaten (void) { return k; }
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
