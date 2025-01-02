// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/executable_path.hpp>
#include <util-generic/syscall.hpp>

#include <boost/filesystem/operations.hpp>

namespace fhg
{
  namespace util
  {
    ::boost::filesystem::path executable_path()
    {
      //! \todo Other systems. See
      //! http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe

      return ::boost::filesystem::canonical ( ::boost::filesystem::path ("/")
                                          / "proc"
                                          / std::to_string (syscall::getpid())
                                          / "exe"
                                          );
    }

    ::boost::filesystem::path executable_path (void* symbol_in_executable)
    try
    {
      return ::boost::filesystem::canonical
        (syscall::dladdr (symbol_in_executable).dli_fname);
    }
    catch (...)
    {
      std::throw_with_nested
        (std::runtime_error ("unable to determine executable_path"));
    }
  }
}
