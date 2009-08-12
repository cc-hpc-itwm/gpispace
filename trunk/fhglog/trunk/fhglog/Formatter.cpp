#include	"Formatter.hpp"

#include <sstream>

using namespace fhg::log;

std::string Formatter::format(const LogEvent &evt)
{
  std::stringstream sstr;
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
  return sstr.str();
}
