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

#include <future>
#include <system_error>

namespace boost
{
  namespace serialization
  {
    template<typename Archive>
      void load (Archive& ar, std::error_code& ec, const unsigned int)
    {
      int value;
      std::string category;
      ar & value;
      ar & category;
      if (category == std::generic_category().name())
      {
        ec = std::error_code (value, std::generic_category());
      }
      else if (category == std::system_category().name())
      {
        ec = std::error_code (value, std::system_category());
      }
      //! \todo Bug in stdlibcxx of gcc 4.8: std::ios_base::failure
      //! does not inherit from std::system_error as defined in c++11,
      //! thus this category is not yet defined.
      // else if (category == std::iostream_category().name())
      // {
      //   ec = std::error_code (value, std::iostream_category());
      // }
      else if (category == std::future_category().name())
      {
        ec = std::error_code (value, std::future_category());
      }
      else
      {
        throw std::logic_error ("unknown std::error_category");
      }
    }
    template<typename Archive>
      void save (Archive& ar, std::error_code const& ec, const unsigned int)
    {
      int const value (ec.value());
      std::string const category (ec.category().name());
      ar & value;
      ar & category;
    }

    template<typename Archive>
      void serialize
        (Archive& ar, std::error_code& ec, const unsigned int version)
    {
      ::boost::serialization::split_free (ar, ec, version);
    }
  }
}
