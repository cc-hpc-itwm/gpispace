// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace iml
{
  namespace beegfs
  {
    template<typename BoostArchive>
      void SegmentDescription::serialize (BoostArchive& archive, unsigned int)
    {
      // Inlined copy of util-generic/serialization/boost/filesystem/path.hpp
      // to avoid dependency in public API.
      ::boost::filesystem::path::string_type path_string;
      if (BoostArchive::is_saving::value)
      {
        path_string = path.string();
      }
      archive & path_string;
      if (BoostArchive::is_loading::value)
      {
        path = path_string;
      }
    }
  }
}
