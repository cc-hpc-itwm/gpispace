// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_TOKENIZER_HPP
#define _EXPR_TOKEN_TOKENIZER_HPP

#include <we/expr/token/prop.hpp>
#include <we/expr/token/type.hpp>

#include <we/expr/exception.hpp>

#include <we/type/literal.hpp>
#include <we/type/literal/read.hpp>

#include <we/type/control.hpp>

#include <we/type/value/container/type.hpp>

#include <fhg/util/show.hpp>

#include <string>
#include <iostream>
#include <vector>

#include <math.h>

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
      literal::type tokval;
      key_vec_t _ref;

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
        return pos.end() || *pos == ';';
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
          {
            ++pos;
          }

        if (is_eof())
          {
            if (!pos.end())
              {
                ++pos;
              }

            token = eof;
          }
        else
          switch (*pos)
            {
            case 'a': ++pos; require ("bs"); unary (abs, "abs"); break;
            case 'b':
              ++pos; require ("itset_");
              if (is_eof())
                throw exception::parse::expected
                  ("'insert', 'delete', 'is_element', 'or', 'tohex' or 'fromhex'", pos());
              else
                switch (*pos)
                  {
                  case 'd': ++pos; require ("elete"); token = _bitset_delete; break;
                  case 'f': ++pos; require ("romhex"); unary (_bitset_fromhex, "bitset_fromhex"); break;
                  case 'i':
                    ++pos;
                    if (is_eof())
                      throw exception::parse::expected
                        ("'nsert' or 's_element'", pos());
                    else
                      switch (*pos)
                        {
                        case 'n':
                          ++pos;
                          require ("sert");
                          token = _bitset_insert;
                          break;
                        case 's':
                          ++pos;
                          require ("_element");
                          token = _bitset_is_element;
                          break;
                        default:
                          throw exception::parse::expected
                            ("'nsert' or 's_element'", pos());
                        }
                    break;
                  case 'o': ++pos; require ("r"); token = _bitset_or; break;
                  case 't': ++pos; require ("ohex"); unary (_bitset_tohex, "bitset_tohex"); break;
                  default:
                    throw exception::parse::expected
                      ("'insert', 'delete', 'is_element', 'or', 'tohex' or 'fromhex'", pos());
                  }
              break;
            case 'c':
              ++pos;
              if (is_eof())
                throw exception::parse::expected
                  ("'os' or 'eil'", pos());
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
              if (is_eof())
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
              if (is_eof())
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
            case 'i': ++pos; require ("f"); token = _if; break;
            case 'l':
              ++pos;
              if (is_eof())
                throw exception::parse::expected
                  ("'en', 'ong' or 'og'", pos());
              else
                switch (*pos)
                  {
                  case 'e': ++pos; require ("n"); unary (_len, "len"); break;
                  case 'o':
                    ++pos;
                    if (is_eof())
                      throw exception::parse::expected ("'ng' or 'g'", pos());
                    else
                      switch (*pos)
                        {
                        case 'n': ++pos; require ("g"); token = _tolong; break;
                        case 'g': ++pos; unary (_log, "log"); break;
                        default: throw exception::parse::expected
                            ("'ng' or 'g'", pos());
                        }
                    break;
                  default:
                    throw exception::parse::expected
                      ("'en', 'ong' or 'og'", pos());
                  }
              break;
            case 'm':
              ++pos;
              if (is_eof())
                throw exception::parse::expected
                  ("'in' or 'ax', 'od' or 'map_...'", pos());
              else
                switch (*pos)
                  {
                  case 'i': ++pos; require ("n"); token = min; break;
                  case 'o': ++pos; require ("d"); token = modint; break;
                  case 'a': ++pos;
                    if (is_eof())
                      throw exception::parse::expected
                        ("'x' or 'p_...'", pos());
                    else
                      switch (*pos)
                        {
                        case 'x': ++pos; token = max; break;
                        case 'p': ++pos; require ("_");
                          if (is_eof())
                            throw exception::parse::expected
                              ("'assign', 'unassign', 'is_assigned' or 'get_assignment'", pos());
                          else
                            switch (*pos)
                              {
                              case 'a': ++pos; require ("ssign"); token = _map_assign; break;
                              case 'u': ++pos; require ("nassign"); token = _map_unassign; break;
                              case 'i': ++pos; require ("s_assigned"); token = _map_is_assigned; break;
                              case 'g': ++pos; require ("et_assignment"); token = _map_get_assignment; break;
                              default:
                                throw exception::parse::expected
                                  ("'assign', 'unassign', 'is_assigned' or 'get_assignment'", pos());
                              }
                          break;
                        default:
                          throw exception::parse::expected
                            ("'x' or 'p_...'", pos());
                        }
                    break;
                  default: throw exception::parse::expected
                      ("'in' or 'ax', 'od' or 'map_...'", pos());
                  }
              break;
            case 'p': ++pos; require("i"); set_PI(); break;
            case 'r': ++pos; require("ound"); token = _round; break;
            case 's':
              ++pos;
              if (is_eof())
                throw exception::parse::expected
                  ("'in', 'ubstr', 'qrt', 'tack_...' or 'et_...'", pos());
              else
                switch (*pos)
                  {
                  case 'i': ++pos; require ("n"); unary (_sin, "sin"); break;
                  case 'q': ++pos; require ("rt"); unary (_sqrt, "sqrt"); break;
                  case 'u': ++pos; require ("bstr"); token = _substr; break;
                  case 'e': ++pos;
                    require ("t_");
                    if (is_eof())
                      throw exception::parse::expected
                        ("'insert', 'erase', 'is_element', 'pop', 'top', 'empty' or 'size'", pos());
                    else
                      switch (*pos)
                        {
                        case 'e': ++pos;
                          if (is_eof())
                            throw exception::parse::expected
                              ("'mpty' or 'rase'", pos());
                          else
                            switch (*pos)
                              {
                              case 'r': ++pos; require ("ase"); token = _set_erase; break;
                              case 'm': ++pos; require ("pty"); unary (_set_empty, "set_empty"); break;
                              default:
                                throw exception::parse::expected
                                  ("'mpty' or 'rase'", pos());
                              }
                          break;
                        case 'i': ++pos;
                          if (is_eof())
                            throw exception::parse::expected
                              ("'nsert' or 's_element'", pos());
                          else
                            switch (*pos)
                              {
                              case 'n': ++pos; require ("sert"); token = _set_insert; break;
                              case 's': ++pos; require ("_element"); token = _set_is_element; break;
                              default:
                                throw exception::parse::expected
                                  ("'nsert' or 's_element'", pos());
                              }
                          break;
                        case 'p': ++pos; require ("op");  unary (_set_pop, "set_pop"); break;
                        case 's': ++pos; require ("ize"); unary (_set_size, "set_size"); break;
                        case 't': ++pos; require ("op");  unary (_set_top, "set_top"); break;
                        default:
                          throw exception::parse::expected
                            ("'insert', 'erase', 'is_element', 'pop_front', 'pop_back', 'top_front' or 'top_back'", pos());
                        }
                    break;
                  case 't':
                    ++pos;
                    require ("ack_");
                    if (is_eof())
                      throw exception::parse::expected
                        ( "'empty', 'top', 'push', 'pop', 'size' or 'join'"
                        , pos()
                        );
                    else
                      switch (*pos)
                        {
                        case 'e': ++pos; require ("mpty");
                          unary (_stack_empty, "stack_empty");
                          break;
                        case 'j': ++pos; require ("oin");
                          token = _stack_join;
                          break;
                        case 's': ++pos; require ("ize");
                          unary (_stack_size, "stack_size");
                          break;
                        case 't': ++pos; require ("op");
                          unary (_stack_top, "stack_top");
                          break;
                        case 'p':
                          ++pos;
                          if (is_eof())
                            throw exception::parse::expected
                              ("'ush' or 'op'", pos());
                          else
                            switch (*pos)
                              {
                              case 'u': ++pos; require ("sh");
                                token = _stack_push;
                                break;
                              case 'o': ++pos; require ("p");
                                unary (_stack_pop, "stack_pop");
                                break;
                              default:
                                throw exception::parse::expected
                                  ("'ush' or 'op'", pos());
                              }
                          break;
                        default:
                          throw exception::parse::expected
                            ( "'empty', 'top', 'push', 'pop', 'size' or 'join'"
                            , pos()
                            );
                        }

                    break;
                  default:
                    throw exception::parse::expected
                      ("'in', 'ubstr', 'qrt', 'tack_...' or 'et_...'", pos());
                  }
              break;
            case 't':
              ++pos;
              if (is_eof())
                throw exception::parse::expected ("'hen' or 'rue'", pos());
              else
                switch (*pos)
                  {
                  case 'h': ++pos; require("en"); token = _then; break;
                  case 'r': ++pos; require("ue"); token = val; tokval = true; break;
                  default: throw exception::parse::expected ("'hen' or 'rue'", pos());
                  }
              break;
            case '|': ++pos; require ("|"); token = _or; break;
            case '&': ++pos; require ("&"); token = _and; break;
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
            case ':':
              ++pos;
              if (is_eof())
                throw exception::parse::expected
                  ("'=', 'eq:', 'ne:', 'lt:', 'le:', 'gt:', 'ge:', 'and:' or 'or:'", pos());
              else
                switch (*pos)
                  {
                  case '=': ++pos; token = define; break;
                  case 'a': ++pos; require ("nd:"); token = _and; break;
                  case 'o': ++pos; require ("r:"); token = _or; break;
                  case 'e': ++pos; require ("q:"); token = eq; break;
                  case 'n': ++pos; require ("e:"); token = ne; break;
                  case 'l':
                    ++pos;
                    if (is_eof())
                      throw exception::parse::expected ("'t:', 'e:'", pos());
                    else
                      switch (*pos)
                        {
                        case 't': ++pos; require (":"); token = lt; break;
                        case 'e': ++pos; require (":"); token = le; break;
                        default:
                          throw exception::parse::expected
                            ("'t:', 'e:'", pos());
                        }
                    break;
                  case 'g':
                    ++pos;
                    if (is_eof())
                      throw exception::parse::expected ("'t:', 'e:'", pos());
                    else
                      switch (*pos)
                        {
                        case 't': ++pos; require (":"); token = gt; break;
                        case 'e': ++pos; require (":"); token = ge; break;
                        default:
                          throw exception::parse::expected
                            ("'t:', 'e:'", pos());
                        }
                    break;
                  default:
                    throw exception::parse::expected
                      ("'=', 'eq:', 'ne:', 'lt:', 'le:', 'gt:', 'ge:', 'and:' or 'or:'", pos());
                  }
              break;
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
              if (!is_eof() && *pos == '*')
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
                    {
                      ++pos;
                      std::string _aref;
                      _ref.clear();
                      while (!pos.end() && *pos != '}')
                        {
                          switch (*pos)
                            {
                            case '.':
                              _ref.push_back (_aref);
                              _aref.clear();
                              break;
                            default:
                              _aref.push_back(*pos);
                              break;
                            }
                          ++pos;
                        }
                      require ("}");
                      _ref.push_back (_aref);
                    }
                    break;
                  default: throw exception::parse::expected ("'{'", pos());
                  }
              break;
            default: token = val; literal::read (tokval, pos); break;
            }
      }

      friend std::ostream & operator << (std::ostream &, const tokenizer &);

    public:
      tokenizer ( std::size_t & _k
                , std::string::const_iterator & _pos
                , const std::string::const_iterator & _end
                )
        : pos (_k, _pos,_end), token (eof) {}
      const literal::type & operator () (void) const { return tokval; }
      const token::type & operator * (void) const { return token; }
      void operator ++ (void) { get(); }
      const key_vec_t & get_ref (void) const { return _ref; }
    };

    static std::string show_key_vec (const key_vec_t & key_vec)
    {
      std::string s;

      for ( key_vec_t::const_iterator pos (key_vec.begin())
          ; pos != key_vec.end()
          ; ++pos
          )
        s += ((pos != key_vec.begin()) ? "." : "") + fhg::util::show (*pos);

      return s;
    }
  }
}

#endif
