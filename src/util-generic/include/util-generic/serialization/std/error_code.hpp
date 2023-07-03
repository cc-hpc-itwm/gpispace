// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
