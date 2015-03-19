// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <fhg/util/indenter.hpp>
#include <util-generic/ostream_modifier.hpp>

#include <boost/optional.hpp>

#include <string>
#include <iosfwd>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      namespace ns
      {
        class open : public ostream::modifier
        {
        public:
          open ( fhg::util::indenter&
               , const boost::optional<std::string>& = boost::none
               );
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
