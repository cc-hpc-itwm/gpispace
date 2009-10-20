#include	"Formatter.hpp"

#include <sstream>
#include <ios>

using namespace fhg::log;

std::string Formatter::format(const LogEvent &evt)
{
  std::stringstream sstr;
  std::string::const_iterator c(fmt_.begin());
  std::string::size_type len(fmt_.size());
  while (len > 0)
  {
    if ('%' == *c)
    {
      // a single % at the end of the format is an error
      if ((len - 1) == 0)
      {
        throw std::runtime_error("Format string ends with a single %");
      }
      else
      {
        // what do we have
        switch (*(c+1))
        {
          case '%':
            sstr << "%";
            break;
          case FMT_SHORT_SEVERITY:
            sstr << evt.severity().str()[0];
            break;
          case FMT_SEVERITY:
            sstr << evt.severity().str();
            break;
          case FMT_FILE:
            sstr << evt.file();
            break;
          case FMT_PATH:
            sstr << evt.path();
            break;
          case FMT_FUNCTION:
            sstr << evt.function();
            break;
          case FMT_LINE:
            sstr << evt.line();
            break;
          case FMT_MESSAGE:
            sstr << evt.message();
            break;
          case FMT_TIMESTAMP:
            sstr << evt.tstamp();
            break;
          case FMT_PID:
            sstr  << evt.pid();
            break;
          case FMT_TID:
            {
              std::ios_base::fmtflags flags(sstr.flags());
              sstr << std::hex << evt.tid();
              sstr.flags(flags);
            }
            break;
          case FMT_LOGGER:
            sstr << evt.logged_via();
            break;
          case FMT_NEWLINE:
            sstr << std::endl;
            break;
          default:
            throw std::runtime_error("Illegal format character occured!");
        }
        ++c; --len;
      }
    }
    else
    {
      sstr << *c;
    }
    ++c; --len;
  }
  return sstr.str();
}
