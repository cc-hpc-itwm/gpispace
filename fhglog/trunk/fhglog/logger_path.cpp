#include "logger_path.hpp"
#include <iterator>
#include <sstream>

using namespace fhg::log;

logger_path::logger_path ()
{}

logger_path::logger_path (const std::string & a_path)
{
  split(a_path, SEPERATOR(), std::back_inserter(path_));
}

logger_path::logger_path (const logger_path & a_path)
  : path_ (a_path.path_)
{}

logger_path::logger_path (const path_type & a_path)
  : path_ (a_path)
{}

std::string logger_path::str(void) const
{
  std::stringstream sstr;

  path_type::const_iterator it (path_.begin());
  if (it != path_.end())
  {
    sstr << *it;
    ++it;
  }

  while (it != path_.end())
  {
    sstr << SEPERATOR() << *it;
    ++it;
  }

  return sstr.str();
}

void logger_path::str(const std::string & s)
{
  path_type p;
  split (s, SEPERATOR(), std::back_inserter(p));
  path_ = p;
}
