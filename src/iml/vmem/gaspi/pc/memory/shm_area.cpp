#include <iml/vmem/gaspi/pc/memory/shm_area.hpp>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <iml/vmem/gaspi/pc/url.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/syscall.hpp>

#include <boost/lexical_cast.hpp>

#include <iml/vmem/gaspi/pc/type/flags.hpp>

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
                          , gpi::pc::type::size_t & size
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

          if (0 == size)
          {
            //! \todo clarify which side opens the segment to not have
            //! this auto-size-determination in here, and to be able
            //! to drop memory_area_t::reinit
            size = fhg::util::syscall::lseek (fd, 0, SEEK_END);
            fhg::util::syscall::lseek (fd, 0, SEEK_SET);
          }
          else if (open_flags & O_CREAT)
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

        static void close (void *ptr, const gpi::pc::type::size_t sz)
        {
          if (ptr)
          {
            fhg::util::syscall::munmap (ptr, sz);
          }
        }
      }

      shm_area_t::shm_area_t ( const gpi::pc::type::process_id_t creator
                             , type::name_t const& name
                             , const gpi::pc::type::size_t user_size
                             , handle_generator_t& handle_generator
                             )
        : area_t ( shm_area_t::area_type
                 , creator
                 , name
                 , user_size
                 , F_NOCREATE | F_EXCLUSIVE
                 , handle_generator
                 )
        , m_ptr (nullptr)
      {
        if (name.empty())
        {
          throw std::runtime_error ("invalid shm name: must not be empty");
        }

        if (name[0] == '/')
          m_path = name;
        else
          m_path = "/" + name;

        gpi::pc::type::size_t size = user_size;

        int open_flags = O_RDWR;

        m_ptr = detail::open ( m_path
                             , size
                             , open_flags
                             , 0600
                             );

        if (0 == user_size)
        {
          descriptor ().local_size = size;
          area_t::reinit ();
        }
      }

      shm_area_t::~shm_area_t()
      {
        try
        {
          detail::close (m_ptr, descriptor().local_size);
          m_ptr = nullptr;
          detail::unlink (m_path);
        }
        catch (...)
        { }
      }

      void*
      shm_area_t::raw_ptr (gpi::pc::type::offset_t off)
      {
        return
          (m_ptr && off < descriptor().local_size)
          ? ((char*)m_ptr + off)
          : nullptr;
      }

      iml_client::vmem::dtmmgr::Arena_t
      shm_area_t::grow_direction (const gpi::pc::type::flags_t) const
      {
        return iml_client::vmem::dtmmgr::ARENA_UP;
      }

      bool
      shm_area_t::is_range_local ( const gpi::pc::type::handle::descriptor_t &hdl
                                 , const gpi::pc::type::offset_t a
                                 , const gpi::pc::type::offset_t b
                                 ) const
      {
        return ((hdl.offset + a) <   size())
          &&   ((hdl.offset + b) <=  size());
      }

      gpi::pc::type::size_t
      shm_area_t::get_local_size ( const gpi::pc::type::size_t sz
                                 , const gpi::pc::type::flags_t
                                 ) const
      {
        return sz;
      }

      double shm_area_t::get_transfer_costs ( const gpi::pc::type::memory_region_t&
                                            , const gpi::rank_t
                                            ) const
      {
        return 0.0;
      }
    }
  }
}