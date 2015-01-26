// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_CPP_STRUCTURE_HPP
#define FHG_UTIL_CPP_STRUCTURE_HPP

#include <fhg/util/indenter.hpp>
#include <fhg/util/ostream_modifier.hpp>

#include <boost/optional.hpp>

#include <string>
#include <iosfwd>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      namespace structure
      {
        class open : public ostream::modifier
        {
        public:
          open (fhg::util::indenter&);
          open (fhg::util::indenter&, const std::string&);
          virtual std::ostream& operator() (std::ostream&) const override;

        private:
          fhg::util::indenter& _indent;
          const boost::optional<std::string> _tag;
        };

        class close : public ostream::modifier
        {
        public:
          close (fhg::util::indenter&);
          virtual std::ostream& operator() (std::ostream&) const override;

        private:
          fhg::util::indenter& _indent;
        };
      }
    }
  }
}

#endif
