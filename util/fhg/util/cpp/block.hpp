// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_CPP_BLOCK_HPP
#define FHG_UTIL_CPP_BLOCK_HPP

#include <fhg/util/indenter.hpp>

#include <boost/optional.hpp>

#include <string>
#include <iosfwd>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      namespace block
      {
        class open
        {
        public:
          open (fhg::util::indenter&);
          open (fhg::util::indenter&, const std::string&);
          std::ostream& operator() (std::ostream&) const;

        private:
          fhg::util::indenter& _indent;
          const boost::optional<std::string> _tag;
        };
        std::ostream& operator<< (std::ostream&, const open&);

        class close
        {
        public:
          close (fhg::util::indenter&);
          std::ostream& operator() (std::ostream&) const;

        private:
          fhg::util::indenter& _indent;
        };
        std::ostream& operator<< (std::ostream&, const close&);
      }
    }
  }
}

#endif
