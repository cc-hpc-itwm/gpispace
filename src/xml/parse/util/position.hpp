// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_POSITION_HPP
#define _XML_PARSE_UTIL_POSITION_HPP

#include <xml/parse/util/position.fwd.hpp>

#include <boost/filesystem.hpp>

#include <iosfwd>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      class position_type
      {
      public:
        position_type ( const char* begin
                      , const char* pos
                      , const boost::filesystem::path&
                      , const unsigned int& line = 1
                      , const unsigned int& column = 0
                      );
        const unsigned int& line() const;
        const unsigned int& column() const;
        const boost::filesystem::path& path() const;

      private:
        unsigned int _line;
        unsigned int _column;
        boost::filesystem::path _path;
      };

      std::ostream& operator<< (std::ostream&, const position_type&);

#define XML_PARSE_UTIL_POSITION_GENERATED()                             \
      ::xml::parse::util::position_type (nullptr, nullptr, __FILE__, __LINE__)
    }
  }
}

#endif
