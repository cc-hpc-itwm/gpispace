// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/filesystem/path.hpp>

namespace fhg
{
  namespace util
  {
    ::boost::filesystem::path executable_path();

    //! Determines executable/library containing the given symbol
    //! rather than the main executable started.
    ::boost::filesystem::path executable_path (void* symbol_in_executable);
    template<typename T>
      ::boost::filesystem::path executable_path (T* symbol_in_executable)
    {
      return executable_path (reinterpret_cast<void*> (symbol_in_executable));
    }
  }
}
