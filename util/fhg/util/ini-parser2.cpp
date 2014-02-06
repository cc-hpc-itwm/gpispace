// mirko.rahn@itwm.fhg.de

#include <fhg/util/ini-parser2.hpp>

#include <fhg/util/parse/ini.hpp>
#include <fhg/util/parse/from_string.hpp>
#include <fhg/util/read_file.hpp>

#include <boost/foreach.hpp>

#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace
    {
      std::map<std::string, std::string>
        to_map (std::list<std::pair<std::string, std::string> > const& l)
      {
        std::map<std::string, std::string> m;

        BOOST_FOREACH
          (std::pair<std::string BOOST_PP_COMMA() std::string> const& kv, l)
        {
          m[kv.first] = kv.second;
        }

        return m;
      }
    }

    ini::ini (boost::filesystem::path const& path)
      : _key_value (to_map (parse::ini_from_string (read_file (path))))
    {}

    boost::optional<std::string const&> ini::get (std::string const& key) const
    {
      std::map<std::string, std::string>::const_iterator const pos
        (_key_value.find (key));

      if (pos != _key_value.end())
      {
        return pos->second;
      }

      return boost::none;
    }
  }
}
