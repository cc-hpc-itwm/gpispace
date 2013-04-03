// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_MKFSTREAM_HPP
#define _XML_PARSE_UTIL_MKFSTREAM_HPP 1

#include <fstream>
#include <sstream>

#include <xml/parse/state.hpp>

#include <boost/filesystem.hpp>

#include <xml/parse/error.hpp>

#include <fhg/util/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      std::ofstream & mk_fstream ( std::ofstream & stream
                                 , const state::type & state
                                 , const boost::filesystem::path & file
                                 );

      class check_no_change_fstream
      {
      public:
        explicit check_no_change_fstream ( const state::type&
                                         , const boost::filesystem::path&
                                         , const bool auto_commit = true
                                         );
        ~check_no_change_fstream();
        void commit() const;

        template<typename T> check_no_change_fstream& operator<< (const T& x)
        {
          _oss << x; return *this;
        }
        check_no_change_fstream& operator<< (std::ostream& (*)(std::ostream&));

      private:
        const state::type& _state;
        const boost::filesystem::path _file;
        std::ostringstream _oss;
        const bool _auto_commit;

        void write() const;
      };
    }
  }
}

#endif
