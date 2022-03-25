// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <future>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      inline std::future_error make_future_error (std::error_code const& ec)
      {
        if (ec.category() != std::future_category())
        {
          throw std::invalid_argument
            ( "called make_future_error with an error_code that is not of "
              "the future category"
            );
        }
        return make_future_error (static_cast<std::future_errc> (ec.value()));
      }
      inline std::future_error make_future_error (std::future_errc const& errc)
      {
        //! \note If you are here because of a compiler error, you are
        //! using a C++11 or C++14 STL implementation that has neither
        //! * a ctor of std::future_errc (the c++17+ one)
        //! * a ctor of std::error_code (the c++11/14 exposition only one)
        //! In c++11/14 there is no standard-compliant way to
        //! construct a std::future_error. Most implementations either
        //! backported the c++17 way or offer an actually exposition
        //! only constructor taking a std::error_code, but not
        //! offering it is still a valid implementation. You will
        //! likely have to look into the future_error implementation
        //! to see if there is another constructor that can be used,
        //! or we can't use this STL implementation.

        //! \note a std::future_errc is implicitly convertible to a
        //! std::error_code, so the actual compatibilty magic is to
        //! always call it with a std::future_errc rather than an
        //! std::error_code in the make_future_error overload
        //! above. Yes, this is an additional category check if we
        //! would have the std::error_code ctor, but is easier to read
        //! code and avoids a compiler detection, and also highlights
        //! the issue that future_error::code() returns something that
        //! can't be used for constructing future_error, even in
        //! c++17.
        return std::future_error (errc);
      }
    }
  }
}
