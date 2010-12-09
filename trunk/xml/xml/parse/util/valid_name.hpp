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
    template<typename Pos>
    inline std::string parse_name (Pos pos)
    {
      std::string name;

      while (!pos.end() && isspace (*pos))
        {
          ++pos;
        }

      if (!pos.end() && (isalpha (*pos) || *pos == '_'))
        {
          name.push_back (*pos);

          ++pos;

          while (!pos.end() && (isalnum (*pos) || *pos == '_'))
            {
              name.push_back (*pos);

              ++pos;
            }
        }

      while (!pos.end() && isspace (*pos))
        {
          ++pos;
        }

      return name;
    }

    inline std::string validate_name ( const std::string & name
                                     , const std::string & type
                                     , const boost::filesystem::path & path
                                     )
    {
      if (parse_name (fhg::util::parse::simple_position (name)) != name)
        {
          throw error::invalid_name (name, type, path);
        }

      return name;
    }
  }
}

#endif
