// Copyright (C) 2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later


  namespace gspc::iml::beegfs
  {
    template<typename BoostArchive>
      void SegmentDescription::serialize (BoostArchive& archive, unsigned int)
    {
      // Inlined copy of util-generic/serialization/std/filesystem/path.hpp
      // to avoid dependency in public API.
      std::filesystem::path::string_type path_string;
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
