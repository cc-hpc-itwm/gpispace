// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_VALID_NAME_HPP
#define _XML_PARSE_VALID_NAME_HPP 1

#include <xml/parse/error.hpp>

#include <boost/filesystem/path.hpp>

#include <string>

#include <fhg/util/parse/position.hpp>

namespace xml
{
  namespace parse
  {
    inline std::string validate_name ( const std::string & name
                                     , const std::string & type
                                     , const boost::filesystem::path & path
                                     )
    {
      if (name.find_first_of (error::invalid_characters) != std::string::npos)
        {
          throw error::invalid_character (name, type, path);
        }

      return name;
    }

    inline std::string
    parse_name (fhg::util::parse::position & pos)
    {
      std::string name;

      while (!pos.end() && isspace (*pos))
        {
          ++pos;
        }

      while (!pos.end() && !isspace (*pos))
        {
          if (error::invalid_characters.find (*pos) == std::string::npos)
            {
              name.push_back (*pos);

              ++pos;
            }
          else
            {
              break;
            }
        }

      while (!pos.end() && isspace (*pos))
        {
          ++pos;
        }

      return name;
    }
  }
}

#endif
