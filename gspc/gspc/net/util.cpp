#include "util.hpp"

#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdexcept>
#include <iterator>

namespace gspc
{
  namespace net
  {
    size_t split_host_port ( std::string const & host_port
                           , std::string & host
                           , std::string & port
                           )
    {
      static const int PARSE_FAILED = -1;
      static const int PARSE_START = 0;
      static const int PARSE_HOST = 1;
      static const int PARSE_PORT = 2;
      static const int PARSE_HOST_BRACKET = 3;
      static const int PARSE_HOST_PORT_SEP = 4;

      // split into host and port
      //  e.g.:
      //     [host]:port -> host, port
      //     host:port   -> host, port

      int mode = PARSE_START;

      host = "";
      port = "";
      std::string::const_iterator       pos = host_port.begin ();
      const std::string::const_iterator end = host_port.end ();

      for (; pos != end && mode != PARSE_FAILED ; ++pos)
      {
        char c = *pos;
        switch (mode)
        {
        case PARSE_START:
          if (c == '[')
          {
            mode = PARSE_HOST_BRACKET;
          }
          else if (isalnum (c))
          {
            mode = PARSE_HOST;
            host.push_back (c);
          }
          else if (c == '*')
          {
            mode = PARSE_HOST_PORT_SEP;
            host.push_back (c);
          }
          else
          {
            mode = PARSE_FAILED;
          }
          break;
        case PARSE_HOST_PORT_SEP:
          if (c == ':')
          {
            mode = PARSE_PORT;
          }
          else
          {
            mode = PARSE_FAILED;
          }
          break;
        case PARSE_HOST:
          if (c == ':')
          {
            mode = PARSE_PORT;
          }
          else if (isalnum (c) || c == '.' || c == '-')
          {
            host.push_back (c);
          }
          else
          {
            mode = PARSE_FAILED;
          }
          break;
        case PARSE_HOST_BRACKET:
          if (c == ']')
          {
            mode = PARSE_HOST_PORT_SEP;
          }
          else if (isalnum (c) || c == ':' || c == '.' || c == '-')
          {
            host.push_back (c);
          }
          else
          {
            mode = PARSE_FAILED;
          }
          break;
        case PARSE_PORT:
          if (isalnum (c))
          {
            port.push_back (c);
          }
          else if (c == '*')
          {
            port = c;
          }
          else
          {
            mode = PARSE_FAILED;
          }
          break;
        default:
          abort ();
        }
      }

      if (  mode == PARSE_FAILED
         || mode == PARSE_HOST_BRACKET
         || (mode == PARSE_PORT && port.empty ())
         )
      {
        return std::distance (host_port.begin (), pos - 1);
      }
      else
      {
        return std::string::npos;
      }
    }

    static std::string s_hostname ()
    {
      char buf [HOST_NAME_MAX];
      int rc = gethostname (buf, sizeof(buf));
      if (0 == rc)
        return std::string (buf);
      else
        return "localhost";
    }

    std::string const &hostname ()
    {
      static std::string h (s_hostname ());
      return h;
    }
  }
}
