// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/filesystem/path.hpp>
#include <boost/serialization/nvp.hpp>

namespace boost
{
  namespace serialization
  {
    template<class Archive>
      void serialize (Archive& ar, ::boost::filesystem::path& path, const unsigned int)
    {
      ::boost::filesystem::path::string_type string;
      if (Archive::is_saving::value)
      {
        string = path.string();
      }
      ar & BOOST_SERIALIZATION_NVP (string);
      if (Archive::is_loading::value)
      {
        path = string;
      }
    }
  }
}
