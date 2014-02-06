//! \todo remove ini-parser and rename ini-parser2 to ini-parser
// mirko.rahn@itwm.fhg.de

#ifndef FHG_UTIL_INI_HPP
#define FHG_UTIL_INI_HPP

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <map>
#include <string>

namespace fhg
{
  namespace util
  {
    class ini
    {
    public:
      ini (boost::filesystem::path const&);

      boost::optional<std::string const&> get (std::string const&) const;
      void put (std::string const&, std::string const&);
      void del (std::string const&);
      std::map<std::string, std::string> const& assignments() const;

    private:
      std::map<std::string, std::string> _key_value;
    };
  }
}

#endif
