// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_POSITION_HPP
#define _XML_PARSE_UTIL_POSITION_HPP

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      class position_type
      {
      public:
        position_type ( const char*
                      , const char*
                      , const boost::filesystem::path&
                      );
        const unsigned int& line() const;
        const unsigned int& column() const;
        const boost::filesystem::path& path() const;
      private:
        unsigned int _line;
        unsigned int _column;
        const boost::filesystem::path _path;
      };
    }
  }
}

#endif
