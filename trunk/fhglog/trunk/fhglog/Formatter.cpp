#include	"Formatter.hpp"

#include <sstream>

using namespace fhg::log;

std::string Formatter::format(const LogEvent &evt)
{
  std::stringstream sstr;
  std::string::const_iterator c(fmt_.begin());
  while (c != fmt_.end())
  {
    if ('%' == *c)
    {
      // a single % at the end of the format is an error
      if ( (c + 1) == fmt_.end())
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
            sstr  << evt.tid();
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
        ++c;
      }
    }
    else
    {
      sstr << *c;
    }
    ++c;
  }
/*
  //
  // fmt = "%t %S thread:%T %f:%L (%F) - %m%n
  sstr << evt.tstamp()
       << " "
       << evt.severity().str()
       << " " 
       << std::hex
       << "thread:" << evt.thread()
       << std::dec
       << " "
       << evt.file()
       << ":" << evt.line()
       << " "
       << "(" << evt.function() << ")"
       << " - " 
       << evt.message()
       << std::endl;
*/
  return sstr.str();
}
