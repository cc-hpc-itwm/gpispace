// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/dynamic_linking.hpp>

#include <utility>

#include <link.h>

namespace fhg
{
  namespace util
  {
    scoped_dlhandle::scoped_dlhandle
        (::boost::filesystem::path const& path, int flags)
      : _ (syscall::dlopen (path.string().c_str(), flags))
    {}
    scoped_dlhandle::~scoped_dlhandle()
    {
      if (_)
      {
        syscall::dlclose (_);
      }
    }

    scoped_dlhandle::scoped_dlhandle (scoped_dlhandle&& other) noexcept
      : _ (std::move (other._))
    {
      other._ = nullptr;
    }

    std::vector<::boost::filesystem::path> currently_loaded_libraries()
    {
      std::vector<::boost::filesystem::path> result;
      dl_iterate_phdr
        ( +[] (dl_phdr_info* info, size_t, void* data)
           {
             static_cast<std::vector<::boost::filesystem::path>*> (data)
               ->emplace_back (info->dlpi_name);
             return 0;
           }
        , &result
        );
      return result;
    }
  }
}
