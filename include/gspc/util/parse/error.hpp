// Copyright (C) 2013-2015,2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdexcept>

#include <gspc/util/parse/position.hpp>

#include <fmt/core.h>




      namespace gspc::util::parse::error
      {
        class generic : public std::runtime_error
        {
        public:
          generic (std::string const& msg, position const& inp)
            : std::runtime_error (inp.error_message (msg))
          {}
        };

        class expected : public generic
        {
        public:
          expected (std::string const&, position const&);
        };

        template<typename From, typename To>
          class value_too_big : public generic
        {
        public:
          value_too_big (From const& f, position const& pos)
            : generic { fmt::format ( "value {} larger than maximum {}"
                                    , f
                                    , std::numeric_limits<To>::max()
                                    )
                      , pos
                      }
          {}
        };

        template<typename I>
          class unexpected_digit : public generic
        {
        public:
          unexpected_digit (position const& pos)
            : generic
              { fmt::format
                ( "unexpected digit (parsed value would be larger than {})"
                , std::numeric_limits<I>::max()
                )
              , pos
              }
          {}
        };
      }
