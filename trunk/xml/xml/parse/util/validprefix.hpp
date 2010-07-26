// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_VALID_PREFIX
#define _XML_PARSE_VALID_PREFIX

#include <xml/parse/error.hpp>

#include <boost/filesystem/path.hpp>

namespace xml
{
  namespace parse
  {
    inline std::string validate_prefix ( const std::string & name
                                       , const std::string & type
                                       , const boost::filesystem::path & path
                                       )
    {
      const std::string::const_iterator end (name.end());
      const std::string::const_iterator pos (name.begin());

      if (pos != end)
        {
          if (*pos == '_')
            {
              throw error::invalid_prefix (name, type, path);
            }
        }

      return name;
    }

    inline bool has_valid_prefix (const std::string & name)
    {
      try
        {
          validate_prefix (name, std::string(), boost::filesystem::path());

          return true;
        }
      catch (const error::invalid_prefix &)
        {
          return false;
        }
    }
  }
}

#endif
