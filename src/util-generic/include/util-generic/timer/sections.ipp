// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <memory>
#include <utility>

namespace fhg
{
  namespace util
  {
    namespace timer
    {
      template<typename Duration, typename Clock>
        sections<Duration, Clock>::sections ( std::string description
                                            , std::ostream& os
                                            )
          : _os (os)
          , _total (std::move (description), _os)
        {}

      template<typename Duration, typename Clock>
        sections<Duration, Clock>::~sections() = default;

      template<typename Duration, typename Clock>
        void sections<Duration, Clock>::end_section()
      {
        _section.reset();
      }

      template<typename Duration, typename Clock>
        void sections<Duration, Clock>::section (std::string description)
      {
        end_section();

        _section = std::make_unique<scoped<Duration, Clock>>
          (std::move (description), _os);
      }
    }
  }
}
