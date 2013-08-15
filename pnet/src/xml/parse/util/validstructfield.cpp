// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/validstructfield.hpp>

#include <xml/parse/util/valid_name.hpp>

#include <xml/parse/error.hpp>

#include <boost/unordered_set.hpp>

namespace xml
{
  namespace parse
  {
    namespace
    {
      struct names_reserved
      {
      public:
        names_reserved()
          : _s()
        {
          _s.insert ("control");
          _s.insert ("bool");
          _s.insert ("int");
          _s.insert ("long");
          _s.insert ("uint");
          _s.insert ("ulong");
          _s.insert ("float");
          _s.insert ("double");
          _s.insert ("char");
        }

        bool operator() (const std::string& x) const
        {
          return _s.find (x) != _s.end();
        }

      private:
        boost::unordered_set<std::string> _s;
      };

    }

    std::string validate_field_name ( const std::string& name
                                    , const boost::filesystem::path& path
                                    )
    {
      static names_reserved reserved;

      if (reserved (name))
      {
        throw error::invalid_field_name (name, path);
      }

      return validate_name (name, "fieldname", path);
    }
  }
}
