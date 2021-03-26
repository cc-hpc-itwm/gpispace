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

#include <iml/vmem/gaspi/pc/segment/segment.hpp>

// needs linking with -lrt
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <util-generic/syscall.hpp>

#include <boost/system/system_error.hpp>

namespace iml
{
  namespace detail
  {
      OpenedSharedMemory::~OpenedSharedMemory()
      {
        close ();
      }

      OpenedSharedMemory::OpenedSharedMemory ( std::string const & name
                                             , const iml::MemorySize sz
                                             )
        : m_ptr (nullptr)
        , _name (name)
        , _size (sz)
      {
        if (name.empty())
          throw std::runtime_error ("invalid name argument to OpenedSharedMemory(): name must not be empty");

        if (_name[0] != '/')
        {
          _name = '/' + _name;
        }
      }

      void OpenedSharedMemory::create (const mode_t mode)
      {
        int fd (-1);

        try
        {
          fd = fhg::util::syscall::shm_open (_name.c_str(), O_RDWR | O_CREAT | O_EXCL, mode);
        }
        catch (boost::system::system_error const&)
        {
          std::throw_with_nested
            ( std::runtime_error
                ("shared memory segment '" + _name + "' could not be created")
            );
        }

        struct close_on_scope_exit
        {
          ~close_on_scope_exit()
          {
            fhg::util::syscall::close (_fd);
          }
          int _fd;
        } _ = {fd};

        try
        {
          fhg::util::syscall::ftruncate (fd, _size);
        }
        catch (boost::system::system_error const&)
        {
          std::throw_with_nested
            ( std::runtime_error
                ( "shared memory segment '" + _name
                + "' could not be truncated to size " + std::to_string (_size)
                )
            );
        }

        try
        {
          m_ptr = fhg::util::syscall::mmap ( nullptr
                                           , _size
                                           , PROT_READ | PROT_WRITE
                                           , MAP_SHARED
                                           , fd
                                           , 0
                                           );
        }
        catch (boost::system::system_error const&)
        {
          std::throw_with_nested
            ( std::runtime_error
                ("shared memory segment '" + _name + "' could not be attached")
            );
        }
      }

      void OpenedSharedMemory::open ()
      {
        int fd (-1);

        try
        {
          fd = fhg::util::syscall::shm_open (_name.c_str(), O_RDWR, 0);
        }
        catch (boost::system::system_error const&)
        {
          std::throw_with_nested
            ( std::runtime_error
                ("shared memory segment '" + _name + "' could not be opened")
            );
        }

        struct close_on_scope_exit
        {
          ~close_on_scope_exit()
          {
            fhg::util::syscall::close (_fd);
          }
          int _fd;
        } _ = {fd};

        try
        {
          m_ptr = fhg::util::syscall::mmap ( nullptr
                                           , _size
                                           , PROT_READ | PROT_WRITE
                                           , MAP_SHARED
                                           , fd
                                           , 0
                                           );
        }
        catch (boost::system::system_error const&)
        {
          std::throw_with_nested
            ( std::runtime_error
                ("shared memory segment '" + _name + "' could not be attached")
            );
        }
      }

      void OpenedSharedMemory::close ()
      {
        if (m_ptr)
        {
          fhg::util::syscall::munmap (m_ptr, _size);
          m_ptr = nullptr;
        }
      }

      void OpenedSharedMemory::unlink ()
      {
        fhg::util::syscall::shm_unlink(_name.c_str());
      }

      void *OpenedSharedMemory::ptr ()
      {
        return m_ptr;
      }

      const void *OpenedSharedMemory::ptr () const
      {
        return m_ptr;
      }
  }
}
