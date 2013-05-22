#include "util.hpp"

#include <errno.h>
#include <stdlib.h> // abort
#include <ctype.h>  // character classes
#include <string.h> // strlen

#include <stack>
#include <iostream>

namespace gspc
{
  namespace rif
  {
    enum parse_state_e
      {
        E_SKIPWS
      , E_SINGLE_QUOTES
      , E_DOUBLE_QUOTES
      , E_ESCAPE
      , E_HEXVALUE_HI
      , E_HEXVALUE_LO
      , E_ARG
      };

    int parse (std::string const &s, std::vector<std::string> &v)
    {
      return parse (s.c_str (), v);
    }

    int parse ( const char *buffer
              , std::vector<std::string> &argv
              )
    {
      return parse (buffer, strlen (buffer), argv);
    }

    int parse ( const char *buffer
              , size_t len
              , std::vector<std::string> &argv
              )
    {
      std::stack<parse_state_e> states;
      states.push (E_SKIPWS);

      char value = 0; // used for \xAB values
      size_t pos = 0;
      std::string arg;

      while (pos < len)
      {
        int mode = states.top ();

        char c = buffer [pos];

        switch (mode)
        {
        case E_SKIPWS:
          if (isspace (c))
            break;

          switch (c)
          {
          case '\'':
            states.push (E_ARG);
            states.push (E_SINGLE_QUOTES);
            break;
          case '\"':
            states.push (E_ARG);
            states.push (E_DOUBLE_QUOTES);
            break;
          case '\\':
            states.push (E_ARG);
            states.push (E_ESCAPE);
            break;
          default:
            arg.push_back (c);
            states.push (E_ARG);
            break;
          }
          break;
        case E_SINGLE_QUOTES:
          switch (c)
          {
          case '\'':
            states.pop ();
            break;
          case '\\':
            states.push (E_ESCAPE);
            break;
          default:
            arg.push_back (c);
          }
          break;
        case E_DOUBLE_QUOTES:
          switch (c)
          {
          case '\"':
            states.pop ();
            break;
          case '\\':
            states.push (E_ESCAPE);
            break;
          default:
            arg.push_back (c);
          }
          break;
        case E_ESCAPE:
          states.pop ();
          switch (c)
          {
          case 'a':
            arg.push_back ('\a');
            break;
          case 'b':
            arg.push_back ('\b');
            break;
          case 'f':
            arg.push_back ('\f');
            break;
          case 'n':
            arg.push_back ('\n');
            break;
          case 'r':
            arg.push_back ('\r');
            break;
          case 't':
            arg.push_back ('\t');
            break;
          case '\\':
            arg.push_back ('\\');
            break;
          case '\'':
            arg.push_back ('\'');
            break;
          case '\"':
            arg.push_back ('\"');
            break;
          case 'x':
            states.push (E_HEXVALUE_LO);
            states.push (E_HEXVALUE_HI);
            break;
          }
          break;
        case E_HEXVALUE_HI:
          states.pop ();
          {
            switch (c)
            {
            case '0'...'9':
              value = ((c - '0') & 0x0f) << 4;
              break;
            case 'a'...'f':
              value = ((c - 'a') & 0x0f) << 4;
              break;
            case 'A'...'F':
              value = ((c - 'A') & 0x0f) << 4;
              break;
            default:
              errno = EINVAL;
              return pos;
            }
          }
          break;
        case E_HEXVALUE_LO:
          states.pop ();
          {
            switch (c)
            {
            case '0'...'9':
              value |= ((c - '0') & 0x0f);
              break;
            case 'a'...'f':
              value |= ((c - 'a') & 0x0f);
              break;
            case 'A'...'F':
              value |= ((c - 'A') & 0x0f);
              break;
            default:
              errno = EINVAL;
              return pos;
            }
          }

          arg.push_back (value);
          value = 0;
          break;
        case E_ARG:
          if (isspace (c))
          {
            argv.push_back (arg);
            arg.clear ();
            states.pop ();
          }
          else
          {
            switch (c)
            {
            case '\'':
              states.push (E_SINGLE_QUOTES);
              break;
            case '\"':
              states.push (E_DOUBLE_QUOTES);
              break;
            case '\\':
              states.push (E_ESCAPE);
              break;
            default:
              arg.push_back (c);
            }
          }
          break;
        default:
          abort ();
        }

        ++pos;
      }

      if (states.top () == E_ARG && not arg.empty ())
      {
        argv.push_back (arg);
        arg.clear ();
      }

      return (int)pos;
    }
  }
}
