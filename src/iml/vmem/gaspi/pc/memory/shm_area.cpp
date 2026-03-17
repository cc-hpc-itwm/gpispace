// Copyright (C) 2011-2012,2014-2016,2019-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/vmem/gaspi/pc/memory/shm_area.hpp>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <exception>
#include <gspc/util/finally.hpp>
#include <gspc/util/print_exception.hpp>
#include <gspc/util/syscall.hpp>
#include <utility>



    namespace gpi::pc::memory
    {
      namespace detail
      {
        static void unlink (std::string const& p)
        {
          gspc::util::syscall::shm_unlink (p.c_str());
        }

        static void* open ( std::string const& path
                          , gspc::iml::MemorySize size
                          , int open_flags
                          , mode_t open_mode = 0
                          )
        {
          int fd (-1);
          void *ptr (nullptr);

          fd = gspc::util::syscall::shm_open
                 (path.c_str(), open_flags, open_mode);
          FHG_UTIL_FINALLY ([fd] { gspc::util::syscall::close (fd); });

          int prot (0);
          if (open_flags & O_RDONLY)
            prot = PROT_READ;
          else if (open_flags & O_WRONLY)
            prot = PROT_WRITE;
          else if (open_flags & O_RDWR)
            prot = PROT_READ | PROT_WRITE;

          if (open_flags & O_CREAT)
          {
            gspc::util::syscall::ftruncate (fd, size);
          }

          try
          {
            ptr = gspc::util::syscall::mmap ( nullptr
                                           , size
                                           , prot
                                           , MAP_SHARED
                                           , fd
                                           , 0
                                           );
          }
          catch (...)
          {
            if (open_flags & O_CREAT)
            {
              detail::unlink (path);
            }

            throw;
          }

          return ptr;
        }

        static void close (void *ptr, gspc::iml::MemorySize sz)
        {
          if (ptr)
          {
            gspc::util::syscall::munmap (ptr, sz);
          }
        }
      }

      shm_area_t::shm_area_t ( type::name_t const& name
                             , gspc::iml::MemorySize user_size
                             )
        : area_t (user_size)
        , _size (user_size)
      {
        if (name.empty())
        {
          throw std::runtime_error ("invalid shm name: must not be empty");
        }

        if (name[0] == '/')
          m_path = name;
        else
          m_path = "/" + name;

        int open_flags = O_RDWR;

        m_ptr = detail::open ( m_path
                             , user_size
                             , open_flags
                             , 0600
                             );
      }

      shm_area_t::~shm_area_t()
      {
        try
        {
          detail::close (m_ptr, _size);
          m_ptr = nullptr;
          detail::unlink (m_path);
        }
        catch (...)
        {
          std::ignore = std::current_exception();
        }
      }

      bool shm_area_t::is_shm_segment() const
      {
        return true;
      }

      void*
      shm_area_t::raw_ptr (gspc::iml::MemoryOffset off)
      {
        return
          (m_ptr && off < _size)
          ? (static_cast<char*> (m_ptr) + off)
          : nullptr;
      }

      bool
      shm_area_t::is_range_local ( gpi::pc::type::handle::descriptor_t const& hdl
                                 , gspc::iml::MemoryOffset a
                                 , gspc::iml::MemoryOffset b
                                 ) const
      {
        return ((hdl.offset + a) <   _size)
          &&   ((hdl.offset + b) <=  _size);
      }

      gspc::iml::MemorySize
      shm_area_t::get_local_size ( gspc::iml::MemorySize sz
                                 , gpi::pc::type::flags_t
                                 ) const
      {
        return sz;
      }

      double shm_area_t::get_transfer_costs ( gspc::iml::MemoryRegion const&
                                            , gpi::rank_t
                                            ) const
      {
        return 0.0;
      }

      global::itopology_t& shm_area_t::global_topology()
      {
        throw std::logic_error
          ("shm_area may never trigger a global operation");
      }
    }
