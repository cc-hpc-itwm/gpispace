#ifndef FHG_ASSERT_HELPER_HPP
#define FHG_ASSERT_HELPER_HPP

#include <string>
#include <sstream>

namespace fhg
{
  namespace assert_helper
  {
    inline std::string message ( std::string const & cond
                               , std::string const & mesg
                               , std::string const & file
                               , int line
                               )
    {
      std::ostringstream sstr;
      sstr << "assertion '" << cond << "'"
           << " in " << file << ":" << line
           << " failed";
      if (not mesg.empty())
      {
        sstr << ": " << mesg;
      }
      return sstr.str();
    }
  };
}

#endif
