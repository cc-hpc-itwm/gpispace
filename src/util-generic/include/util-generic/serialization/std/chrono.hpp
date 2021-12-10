// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <boost/serialization/split_free.hpp>

#include <chrono>

namespace boost
{
  namespace serialization
  {
    template<class Archive, class Rep, class Period>
      void load ( Archive& ar
                , std::chrono::duration<Rep, Period>& duration
                , const unsigned int
                )
    {
      Rep rep;
      ar & rep;
      duration = std::chrono::duration<Rep, Period> (rep);
    }

    template<class Archive, class Rep, class Period>
      void save ( Archive& ar
                , std::chrono::duration<Rep, Period> const& duration
                , const unsigned int
                )
    {
      Rep const rep (duration.count());
      ar & rep;
    }

    template<class Archive, class Rep, class Period>
      void serialize ( Archive& ar
                     , std::chrono::duration<Rep, Period>& duration
                     , const unsigned int version
                     )
    {
      ::boost::serialization::split_free (ar, duration, version);
    }


    template<class Archive, class Clock, class Duration>
      void load ( Archive& ar
                , std::chrono::time_point<Clock, Duration>& time_point
                , const unsigned int
                )
    {
      Duration since_epoch;
      ar & since_epoch;
      time_point = std::chrono::time_point<Clock, Duration> (since_epoch);
    }

    template<class Archive, class Clock, class Duration>
      void save ( Archive& ar
                , std::chrono::time_point<Clock, Duration> const& time_point
                , const unsigned int
                )
    {
      ar & time_point.time_since_epoch();
    }

    template<class Archive, class Clock, class Duration>
      void serialize ( Archive& ar
                     , std::chrono::time_point<Clock, Duration>& time_point
                     , const unsigned int version
                     )
    {
      ::boost::serialization::split_free (ar, time_point, version);
    }
  }
}
