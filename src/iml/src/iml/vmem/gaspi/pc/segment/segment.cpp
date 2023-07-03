// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/vmem/gaspi/pc/segment/segment.hpp>

// needs linking with -lrt
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <util-generic/syscall.hpp>

#include <boost/system/system_error.hpp>

namespace iml
{
  namespace detail
  {
    namespace
    {
      struct close_on_scope_exit
      {
        close_on_scope_exit (int fd)
          : _fd {fd}
        {}
        ~close_on_scope_exit()
        {
          fhg::util::syscall::close (_fd);
        }
        close_on_scope_exit (close_on_scope_exit const&) = delete;
        close_on_scope_exit& operator= (close_on_scope_exit const&) = delete;
        close_on_scope_exit (close_on_scope_exit&&) = delete;
        close_on_scope_exit& operator= (close_on_scope_exit&&) = delete;

        int _fd;
      };
    }

      OpenedSharedMemory::~OpenedSharedMemory()
      {
        close ();
      }

      OpenedSharedMemory::OpenedSharedMemory ( std::string const& name
                                             , iml::MemorySize sz
                                             )
        : _name (name)
        , _size (sz)
      {
        if (name.empty())
          throw std::runtime_error ("invalid name argument to OpenedSharedMemory(): name must not be empty");

        if (_name[0] != '/')
        {
          _name = '/' + _name;
        }
      }

      void OpenedSharedMemory::create (mode_t mode)
      {
        int fd (-1);

        try
        {
          fd = fhg::util::syscall::shm_open (_name.c_str(), O_RDWR | O_CREAT | O_EXCL, mode);
        }
        catch (::boost::system::system_error const&)
        {
          std::throw_with_nested
            ( std::runtime_error
                ("shared memory segment '" + _name + "' could not be created")
            );
        }

        close_on_scope_exit const _ {fd};

        try
        {
          fhg::util::syscall::ftruncate (fd, _size);
        }
        catch (::boost::system::system_error const&)
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
        catch (::boost::system::system_error const&)
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
        catch (::boost::system::system_error const&)
        {
          std::throw_with_nested
            ( std::runtime_error
                ("shared memory segment '" + _name + "' could not be opened")
            );
        }

        close_on_scope_exit const _ {fd};

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
        catch (::boost::system::system_error const&)
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
