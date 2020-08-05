#pragma once

#include <fhg/util/indenter.hpp>
#include <util-generic/ostream/modifier.hpp>

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
        class open : public ostream::modifier
        {
        public:
          open ( fhg::util::indenter&
               , const boost::optional<std::string>& = boost::none
               );
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
