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

#include <util-generic/syscall/directory.hpp>

#include <util-generic/syscall.hpp>

namespace fhg
{
  namespace util
  {
    namespace syscall
    {
      directory::directory (boost::filesystem::path const& path)
        : _ (syscall::opendir (path.string().c_str()))
      {}
      directory::~directory()
      {
        syscall::closedir (static_cast<DIR*> (_));
      }

      int directory::fd() const
      {
        return syscall::dirfd (static_cast<DIR*> (_));
      }
    }
  }
}
