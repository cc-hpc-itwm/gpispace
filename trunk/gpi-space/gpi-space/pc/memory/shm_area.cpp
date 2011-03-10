#include "shm_area.hpp"

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <fhglog/minimal.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      shm_area_t::shm_area_t ( const gpi::pc::type::id_t id
                             , const gpi::pc::type::process_id_t creator
                             , const std::string & path
                             , const gpi::pc::type::size_t size
                             , const gpi::pc::type::flags_t flags
                             )
          : area_t ( gpi::pc::type::segment::SEG_SHM
                   , id
                   , creator
                   , path
                   , size
                   , flags
                   )
          , m_ptr (NULL)
      {
        m_ptr = shm_area_t::open ( path
                                 , size
                                 , O_RDWR // TODO: pass via flags
                                 );
        if (unlink_after_open (flags))
        {
          shm_area_t::unlink (path);
        }
      }

     shm_area_t::~shm_area_t()
     {
       try
       {
         shm_area_t::close(m_ptr, descriptor().size);
         m_ptr = 0;
         if (unlink_after_close (descriptor().flags))
         {
           shm_area_t::unlink (descriptor().name);
         }
       }
       catch (std::exception const & ex)
       {
         LOG( ERROR
            , "error in ~shm_area_t:"
            << " id = " << descriptor().id
            << " ptr = " << m_ptr
            << " path = " << descriptor().name
            << " error = " << ex.what()
            );
       }
     }

     bool
     shm_area_t::is_allowed_to_attach
        (const gpi::pc::type::process_id_t proc) const
     {
       if (gpi::flag::is_set
           (descriptor ().flags, gpi::pc::type::segment::F_EXCLUSIVE))
       {
         if (proc == descriptor ().creator)
           return true;
         else
           return false;
       }
       return true;
     }

     area_t::grow_direction_t
     shm_area_t::grow_direction (const gpi::pc::type::flags_t) const
     {
        return area_t::GROW_UP;
     }

      void
      shm_area_t::check_bounds ( const gpi::pc::type::handle::descriptor_t &hdl
                               , const gpi::pc::type::offset_t start
                               , const gpi::pc::type::offset_t end
                               ) const
      {
        if (! (start < hdl.size && end < hdl.size))
        {
          throw std::invalid_argument
              ("out-of-bounds: access to shm handle outside boundaries");
        }
      }

     bool shm_area_t::unlink_after_open (const gpi::pc::type::flags_t flgs)
     {
       if (gpi::flag::is_set (flgs, gpi::pc::type::segment::F_EXCLUSIVE))
         return true;
       return false;
     }

     bool shm_area_t::unlink_after_close (const gpi::pc::type::flags_t flgs)
     {
       if (gpi::flag::is_set (flgs, gpi::pc::type::segment::F_NOUNLINK))
         return false;
       if (gpi::flag::is_set (flgs, gpi::pc::type::segment::F_EXCLUSIVE))
         return false;
       return true;
     }

     void* shm_area_t::open ( std::string const & path
                            , const gpi::pc::type::size_t size
                            , const int open_flags
                            , const mode_t open_mode
                            )
     {
       int err (0);
       int fd (-1);
       void *ptr (0);

       fd = shm_open (path.c_str(), open_flags, open_mode);
       if (fd < 0)
       {
         std::string err (strerror(errno));
         throw std::runtime_error ("open: " + err);
       }

       int prot (0);
       if (open_flags & O_RDONLY)
         prot = PROT_READ;
       else if (open_flags & O_WRONLY)
         prot = PROT_WRITE;
       else if (open_flags & O_RDWR)
         prot = PROT_READ | PROT_WRITE;

       ptr = mmap ( NULL
                  , size
                  , prot
                  , MAP_SHARED
                  , fd
                  , 0
                  );
       if (MAP_FAILED == ptr)
       {
         std::string err (strerror(errno));
        ::close (fd);
         throw std::runtime_error ("mmap: " + err);
       }

       ::close (fd);
       return ptr;
     }

     void shm_area_t::close ( void *ptr
                            , const gpi::pc::type::size_t sz
                            )
     {
       if (ptr)
       {
         if (munmap(ptr, sz) < 0)
         {
           std::string err (strerror(errno));
           throw std::runtime_error ("munmap: " + err);
         }
       }
     }

     void shm_area_t::unlink (std::string const & p)
     {
       if (shm_unlink (p.c_str()) < 0)
       {
         std::string err (strerror(errno));
         throw std::runtime_error ("unlink: " + err);
       }
     }

     bool
     shm_area_t::is_range_local ( const gpi::pc::type::handle::descriptor_t &hdl
                                , const gpi::pc::type::offset_t a
                                , const gpi::pc::type::offset_t b
                                ) const
     {
       return ((hdl.offset + a) < size())
           && ((hdl.offset + b) < size());
     }
    }
  }
}
