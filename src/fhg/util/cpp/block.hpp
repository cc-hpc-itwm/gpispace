// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <fhg/util/indenter.hpp>
#include <util-generic/ostream/modifier.hpp>

#include <boost/optional.hpp>

#include <iosfwd>
#include <string>

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
               , ::boost::optional<std::string> const& = ::boost::none
               );
          std::ostream& operator() (std::ostream&) const override;

        private:
          fhg::util::indenter& _indent;
          const ::boost::optional<std::string> _tag;
        };

        class close : public ostream::modifier
        {
        public:
          close (fhg::util::indenter&);
          std::ostream& operator() (std::ostream&) const override;

        private:
          fhg::util::indenter& _indent;
        };
      }
    }
  }
}
