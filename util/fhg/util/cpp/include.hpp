// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <fhg/util/ostream_modifier.hpp>

#include <boost/filesystem.hpp>

#include <iosfwd>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      class include : public ostream::modifier
      {
      public:
        include (const std::string&);
        virtual std::ostream& operator() (std::ostream&) const override;
      private:
        const std::string& _fname;
      };
    }
  }
}
