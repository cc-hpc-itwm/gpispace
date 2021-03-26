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

#include <iml/vmem/gaspi/pc/memory/shm_area.hpp>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <util-generic/finally.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/syscall.hpp>

#include <boost/lexical_cast.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      namespace detail
      {
        static void unlink (std::string const &p)
        {
          fhg::util::syscall::shm_unlink (p.c_str());
        }

        static void* open ( std::string const & path
                          , iml::MemorySize size
                          , const int open_flags
                          , const mode_t open_mode = 0
                          )
        {
          int fd (-1);
          void *ptr (nullptr);

          fd = fhg::util::syscall::shm_open
                 (path.c_str(), open_flags, open_mode);
          FHG_UTIL_FINALLY ([fd] { fhg::util::syscall::close (fd); });

          int prot (0);
          if (open_flags & O_RDONLY)
            prot = PROT_READ;
          else if (open_flags & O_WRONLY)
            prot = PROT_WRITE;
          else if (open_flags & O_RDWR)
            prot = PROT_READ | PROT_WRITE;

          if (open_flags & O_CREAT)
          {
            fhg::util::syscall::ftruncate (fd, size);
          }

          try
          {
            ptr = fhg::util::syscall::mmap ( nullptr
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
              detail::unlink (path.c_str ());
            }

            throw;
          }

          return ptr;
        }

        static void close (void *ptr, const iml::MemorySize sz)
        {
          if (ptr)
          {
            fhg::util::syscall::munmap (ptr, sz);
          }
        }
      }

      shm_area_t::shm_area_t ( type::name_t const& name
                             , const iml::MemorySize user_size
                             )
        : area_t (user_size)
        , m_ptr (nullptr)
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
        { }
      }

      bool shm_area_t::is_shm_segment() const
      {
        return true;
      }

      void*
      shm_area_t::raw_ptr (iml::MemoryOffset off)
      {
        return
          (m_ptr && off < _size)
          ? (static_cast<char*> (m_ptr) + off)
          : nullptr;
      }

      bool
      shm_area_t::is_range_local ( const gpi::pc::type::handle::descriptor_t &hdl
                                 , const iml::MemoryOffset a
                                 , const iml::MemoryOffset b
                                 ) const
      {
        return ((hdl.offset + a) <   _size)
          &&   ((hdl.offset + b) <=  _size);
      }

      iml::MemorySize
      shm_area_t::get_local_size ( const iml::MemorySize sz
                                 , const gpi::pc::type::flags_t
                                 ) const
      {
        return sz;
      }

      double shm_area_t::get_transfer_costs ( const iml::MemoryRegion&
                                            , const gpi::rank_t
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
  }
}
