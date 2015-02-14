// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <fstream>
#include <sstream>

#include <xml/parse/state.hpp>

#include <boost/filesystem.hpp>

#include <xml/parse/error.hpp>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      class check_no_change_fstream
      {
      public:
        explicit check_no_change_fstream ( const state::type&
                                         , const boost::filesystem::path&
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

        void write() const;
      };
    }
  }
}
