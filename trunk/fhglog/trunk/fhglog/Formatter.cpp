#include	"Formatter.hpp"

#include <sstream>

using namespace fhg::log;

const std::string Formatter::FMT_SEVERITY = "S";
const std::string Formatter::FMT_FILE     = "f";
const std::string Formatter::FMT_FUNCTION = "F";
const std::string Formatter::FMT_LINE     = "L";
const std::string Formatter::FMT_MESSAGE  = "m";
const std::string Formatter::FMT_TIMESTAMP= "t";
const std::string Formatter::FMT_THREAD   = "T";
const std::string Formatter::FMT_NEWLINE  = "n";

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
