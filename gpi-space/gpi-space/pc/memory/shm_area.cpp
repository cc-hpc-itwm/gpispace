#include "shm_area.hpp"

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <fhglog/minimal.hpp>
#include <fhg/util/url.hpp>
#include <fhg/util/read_bool.hpp>

#include <boost/lexical_cast.hpp>

#include <gpi-space/pc/type/flags.hpp>

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
          shm_unlink (p.c_str());
        }

        static void* open ( std::string const & path
                          , gpi::pc::type::size_t & size
                          , const int open_flags
                          , const mode_t open_mode = 0
                          )
        {
          int fd (-1);
          void *ptr (0);

          fd = shm_open (path.c_str(), open_flags, open_mode);
          if (fd < 0)
          {
            std::string err = "open: " + path + ": " + strerror (errno);
            throw std::runtime_error (err);
          }

          int prot (0);
          if (open_flags & O_RDONLY)
            prot = PROT_READ;
          else if (open_flags & O_WRONLY)
            prot = PROT_WRITE;
          else if (open_flags & O_RDWR)
            prot = PROT_READ | PROT_WRITE;

          if (0 == size)
          {
            off_t end = lseek (fd, 0, SEEK_END);
            if (end == (off_t)(-1))
            {
              std::string err (strerror (errno));
              ::close (fd);
              throw std::runtime_error ("lseek: " + err);
            }
            else
            {
              size = (gpi::pc::type::size_t)(end);
              lseek (fd, 0, SEEK_SET);
            }
          }
          else if (open_flags & O_CREAT)
          {
            if (ftruncate (fd, size) != 0)
            {
              std::string err (strerror (errno));
              ::close (fd); fd = -1;
              throw std::runtime_error ("ftruncate: " + err);
            }
          }

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

            if (open_flags & O_CREAT)
            {
              detail::unlink (path.c_str ());
            }

            throw std::runtime_error ("mmap: " + err);
          }

          ::close (fd);
          return ptr;
        }

        static void close (void *ptr, const gpi::pc::type::size_t sz)
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
      }

      shm_area_t::shm_area_t ( const gpi::pc::type::process_id_t creator
                             , const std::string & name
                             , const gpi::pc::type::size_t user_size
                             , const gpi::pc::type::flags_t flags
                             )
        : area_t ( shm_area_t::area_type
                 , creator
                 , name
                 , user_size
                 , flags
                 )
        , m_ptr (NULL)
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

        if (not gpi::flag::is_set (flags, gpi::pc::F_NOCREATE))
        {
          MLOG (INFO, "setting open_flags to O_CREAT + O_EXCL");
          open_flags |= O_CREAT | O_EXCL;
        }

        m_ptr = detail::open ( m_path
                             , size
                             , open_flags
                             , 0600
                             );
        if (unlink_after_open (flags))
        {
          detail::unlink (m_path);
        }

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
          m_ptr = 0;
          if (unlink_after_close (descriptor().flags))
          {
            detail::unlink (m_path);
          }
        }
        catch (std::exception const & ex)
        {
          LOG( ERROR
             , "error in ~shm_area_t:"
             << " id = " << descriptor().id
             << " ptr = " << m_ptr
             << " path = " << m_path
             << " error = " << ex.what()
             );
        }
      }

      void*
      shm_area_t::raw_ptr (gpi::pc::type::offset_t off)
      {
        return
          (m_ptr && off < descriptor().local_size)
          ? ((char*)m_ptr + off)
          : 0;
      }

      area_t::grow_direction_t
      shm_area_t::grow_direction (const gpi::pc::type::flags_t) const
      {
        return area_t::GROW_UP;
      }

      void
      shm_area_t::check_bounds ( const gpi::pc::type::handle::descriptor_t &hdl
                               , const gpi::pc::type::offset_t start
                               , const gpi::pc::type::offset_t amount
                               ) const
      {
        if (! (start < hdl.size && (start + amount) <= hdl.size))
        {
          CLOG( ERROR
              , "shm.memory"
              , "out-of-bounds access:"
              << " hdl=" << hdl
              << " size=" << hdl.size
              << " range=["<<start << ", " << start+amount << "]"
              );
          throw std::invalid_argument
            ( std::string("out-of-bounds:")
            + " access to shm handle"
            + " " + boost::lexical_cast<std::string>(hdl.id)
            +" outside boundaries"
            + " ["
            + boost::lexical_cast<std::string>(start)
            + ","
            + boost::lexical_cast<std::string>(start+amount)
            + ")"
            + " is not within [0, " + boost::lexical_cast<std::string>(hdl.size) + ")"
            );
        }
      }

      bool shm_area_t::unlink_after_open (const gpi::pc::type::flags_t flgs)
      {
        return false;
      }

      bool shm_area_t::unlink_after_close (const gpi::pc::type::flags_t flgs)
      {
        if (gpi::flag::is_set (flgs, gpi::pc::F_PERSISTENT))
          return false;
        return true;
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

      int
      shm_area_t::get_specific_transfer_tasks ( const gpi::pc::type::memory_location_t src
                                              , const gpi::pc::type::memory_location_t dst
                                              , area_t & dst_area
                                              , gpi::pc::type::size_t amount
                                              , gpi::pc::type::size_t queue
                                              , task_list_t & tasks
                                              )
      {
        LOG ( ERROR
            , "specific transfer not implemented: "
            << amount << " bytes: "
            << dst
            << " <- "
            << src
            );
        throw std::runtime_error
          ("get_specific_transfer_tasks not implemented on shm_area");
      }

      area_ptr_t shm_area_t::create (std::string const &url_s)
      {
        using namespace fhg::util;
        using namespace gpi::pc;

        url_t url (url_s);
        gpi::pc::type::flags_t flags = F_NONE;

        if (not read_bool (url.get ("create", "false")))
        {
          gpi::flag::set (flags, F_NOCREATE);
        }
        if (    read_bool (url.get ("unlink", "false")))
        {
          gpi::flag::set (flags, F_FORCE_UNLINK);
        }
        if (not read_bool (url.get ("mmap", "false")))
        {
          gpi::flag::set (flags, F_NOMMAP);
        }
        if (    read_bool (url.get ("exclusive", "false")))
        {
          gpi::flag::set (flags, F_EXCLUSIVE);
        }
        if (    read_bool (url.get ("persistent", "false")))
        {
          gpi::flag::set (flags, F_PERSISTENT);
        }

        gpi::pc::type::size_t size =
          boost::lexical_cast<gpi::pc::type::size_t>(url.get ("size", "0"));

        area_ptr_t area (new shm_area_t ( GPI_PC_INVAL
                                        , url.path ()
                                        , size
                                        , flags
                                        )
                        );
        return area;
      }
    }
  }
}
