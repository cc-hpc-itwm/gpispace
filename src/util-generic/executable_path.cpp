// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <util-generic/executable_path.hpp>
#include <util-generic/syscall.hpp>

#include <boost/filesystem/operations.hpp>

namespace fhg
{
  namespace util
  {
    boost::filesystem::path executable_path()
    {
      //! \todo Other systems. See
      //! http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe

      return boost::filesystem::canonical ( boost::filesystem::path ("/")
                                          / "proc"
                                          / std::to_string (syscall::getpid())
                                          / "exe"
                                          );
    }

    boost::filesystem::path executable_path (void* symbol_in_executable)
    try
    {
      return boost::filesystem::canonical
        (syscall::dladdr (symbol_in_executable).dli_fname);
    }
    catch (...)
    {
      std::throw_with_nested
        (std::runtime_error ("unable to determine executable_path"));
    }
  }
}
