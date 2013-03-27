// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/util/valid_name.hpp>

#include <xml/parse/error.hpp>

namespace xml
{
  namespace parse
  {
    std::string parse_name (fhg::util::parse::position pos)
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
      std::size_t k (0);
      std::string::const_iterator begin (name.begin());
      const std::string::const_iterator end (name.end());
      if (parse_name (fhg::util::parse::position (k, begin, end)) != name)
      {
        throw error::invalid_name (name, type, path);
      }

      return name;
    }
  }
}
