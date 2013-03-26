// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/validprefix.hpp>

#include <xml/parse/error.hpp>

#include <rewrite/validprefix.hpp>

namespace xml
{
  namespace parse
  {
    std::string validate_prefix ( const std::string& name
                                , const std::string& type
                                , const boost::filesystem::path& path
                                )
    {
      if (rewrite::has_magic_prefix (name))
        {
          throw error::invalid_prefix (name, type, path);
        }

      return name;
    }
  }
}
