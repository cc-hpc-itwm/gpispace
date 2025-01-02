// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/syscall/directory.hpp>

#include <util-generic/syscall.hpp>

namespace fhg
{
  namespace util
  {
    namespace syscall
    {
      directory::directory (::boost::filesystem::path const& path)
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
