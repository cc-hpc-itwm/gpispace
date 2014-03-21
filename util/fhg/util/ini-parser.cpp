// mirko.rahn@itwm.fhg.de

#include <fhg/util/ini-parser.hpp>

#include <fhg/util/parse/ini.hpp>
#include <fhg/util/parse/from_string.hpp>
#include <fhg/util/read_file.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace fhg
{
  namespace util
  {
    ini::ini (boost::filesystem::path const& path)
      : _key_value (parse::ini_map (read_file (path)))
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

    void ini::put (std::string const& key, std::string const& value)
    {
      if (!_key_value.insert (std::make_pair (key, value)).second)
      {
        throw std::runtime_error
          ( boost::str
            ( boost::format
              ( "ini: try to overwrite key '%1%'"
              " with value '%2%', old value is '%3%'"
              ) % key % value % _key_value.at (key)
            )
          );
      }
    }

    void ini::del (std::string const& key)
    {
      _key_value.erase (key);
    }

    std::map<std::string, std::string> const& ini::assignments() const
    {
      return _key_value;
    }
  }
}
