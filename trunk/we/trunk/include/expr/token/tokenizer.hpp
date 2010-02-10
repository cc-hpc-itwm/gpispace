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
          eat();

        if (pos == end)
          token = eof;
        else
          switch (*pos)
            {
            case 'a':
              eat();
              if (pos == end || *pos != 'b')
                throw expected ("bs", k);
              else
                {
                  eat();
                  if (pos == end || *pos != 's')
                    throw expected ("s", k);
                  else
                    {
                      eat();
                      if (next_can_be_unary (token))
                        token = abs;
                      else
                        throw misplaced ("abs", k);
                    }
                }
              break;
            case 'c':
              eat();
              if (pos == end || *pos != 'o')
                token = com;
              else
                {
                  eat();
                  if (pos == end || *pos != 's')
                    throw expected ("s", k);
                  else
                    {
                      eat();
                      if (next_can_be_unary (token))
                        token = _cos;
                      else
                        throw misplaced ("cos", k);
                    }
                }
              break;
            case 'e': eat(); token = val; tokval = 2.7182818284590452354; break;
            case 'f': eat(); token = fac; break;
            case 'l':
              eat();
              if (pos == end || *pos != 'o')
                throw expected ("og", k);
              else
                {
                  eat();
                  if (pos == end || *pos != 'g')
                    throw expected ("g", k);
                  else
                    {
                      eat();
                      if (next_can_be_unary (token))
                        token = _log;
                      else
                        throw misplaced ("log", k);
                    }
                }
              break;
            case 'm':
              eat();
              if (pos == end)
                throw expected ("in or ax", k);
              else
                switch (*pos)
                  {
                  case 'i':
                    eat();
                    if (pos == end)
                      throw expected ("n", k);
                    else
                      if (*pos == 'n')
                        {
                          eat();
                          token = min;
                        }
                      else
                        throw expected ("n", k);
                    break;
                  case 'a':
                    eat();
                    if (pos == end)
                      throw expected ("x", k);
                    else
                      if (*pos == 'x')
                        {
                          eat();
                          token = max;
                        }
                      else
                        throw expected ("x", k);
                    break;
                  default: throw expected ("in or ax", k);
                  }
              break;
            case 'p':
              eat();
              if (pos == end || *pos != 'i')
                throw expected ("i", k);
              else
                {
                  eat();
                  token = val;
                  tokval = 3.14159265358979323846;
                }
              break;
            case 's':
              eat();
              if (pos == end)
                throw expected ("'in' or 'qrt'", k);
              else
                switch (*pos)
                  {
                  case 'i':
                    eat();
                    if (pos == end || *pos != 'n')
                      throw expected ("n", k);
                    else
                      {
                        eat();
                        if (next_can_be_unary (token))
                          token = _sin;
                        else
                          throw misplaced ("sin", k);
                      }
                    break;
                  case 'q':
                    eat();
                    if (pos == end || *pos != 'r')
                      throw expected ("rt", k);
                    else
                      {
                        eat();
                        if (pos == end || *pos != 't')
                          throw expected ("t", k);
                        else
                          {
                            eat();
                            if (next_can_be_unary (token))
                              token = _sqrt;
                            else
                              throw misplaced ("sqrt", k);
                          }
                      }
                    break;
                  default: throw expected ("'in' or 'qrt'", k);
                  }
              break;
            case '|': eat(); token = _or; break;
            case '&': eat(); token = _and; break;
            case '<':
              eat();
              if (pos == end)
                token = lt;
              else
                switch (*pos)
                  {
                  case '=': eat(); token = le; break;
                  default: token = lt; break;
                  }
              break;
            case '>':
              eat();
              if (pos == end)
                token = gt;
              switch (*pos)
                {
                case '=': eat(); token = ge; break;
                default: token = gt; break;
                }
              break;
            case '!':
              eat();
              if (pos == end)
                throw expected("= or expression", k);
              else
                switch (*pos)
                  {
                  case '=': eat(); token = ne; break;
                  default:
                    if (next_can_be_unary (token))
                      token = _not;
                    else
                      throw misplaced ("negation", k);
                    break;
                  }
              break;
            case '=':
              eat();
              if (pos == end)
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
              if (pos == end)
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
                    if (*pos != '}')
                      throw expected ("}", k);
                    eat();
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
      tokenizer (const std::string & input) 
        : k (0), pos (input.begin()), end (input.end()), token (eof) {}
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
