// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/util/valid_name.hpp>

#include <xml/parse/error.hpp>

namespace xml
{
  namespace parse
  {
    std::string parse_name (fhg::util::parse::position& pos)
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

    std::string validate_name ( const std::string & name
                              , const std::string & type
                              , const boost::filesystem::path & path
                              )
    {
      fhg::util::parse::position pos (name);

      if (parse_name (pos) != name)
      {
        throw error::invalid_name (name, type, path);
      }

      return name;
    }
  }
}
