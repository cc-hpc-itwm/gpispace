#include "segment.hpp"

// needs linking with -lrt
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
    namespace segment
    {
      segment_t::~segment_t()
      {
        try
        {
          close ();
        }
        catch (std::exception const & ex)
        {
          LOG(WARN, "could not stop shared segment: " << name());
        }
      }

      segment_t::segment_t ( std::string const & nme
                           , type::size_t sz
                           , type::segment_id_t id
                           )
        : m_ptr (0)
      {
        if (nme.empty())
          throw std::runtime_error ("invalid name argument to segment_t(): name must not be empty");

        if (nme[0] != '/')
          m_descriptor.name = "/" + nme;
        else
          m_descriptor.name = nme;
        m_descriptor.size = sz;
        m_descriptor.id = id;
      }

      void segment_t::create (bool persistent)
      {
        int err (0);
        int fd (-1);

        fd = shm_open (name().c_str(), O_RDWR | O_CREAT | O_EXCL, S_IRWXU);
        if (fd < 0)
        {
          err = errno;
          LOG(ERROR, "shared memory segment: " << name() << " could not be created: " << strerror(err));
          throw std::runtime_error ("shared memory segment could not be created");
        }

        err = ftruncate (fd, size());
        if (err < 0)
        {
          err = errno;
          LOG( ERROR
             , "shared memory segment: " << name()
             << " could not be truncated to size: " << size()
             << ": " << strerror(err)
             );
          ::close (fd);
          throw std::runtime_error
            ("shared memory segment could not be truncated");
        }

        m_ptr = mmap ( NULL
                     , size()
                     , PROT_READ | PROT_WRITE
                     , MAP_SHARED
                     , fd
                     , 0
                     );
        if (m_ptr == (void*)-1)
        {
          err = errno;
          LOG(ERROR, "shared memory segment: " << name() << " could not be attached: " << strerror(err));
          ::close (fd);
          throw std::runtime_error ("shared memory segment could not be attached");
        }

        ::close (fd);
        LOG(INFO, "shared memory segment " << name() << " created: " << m_ptr);

        m_persistent = persistent;
      }

      void segment_t::open ()
      {
        int err (0);
        int fd (-1);

        fd = shm_open (name().c_str(), O_RDWR, 0);
        if (fd < 0)
        {
          err = errno;
          LOG(ERROR, "shared memory segment: " << name() << " could not be opened: " << strerror(err));
          throw std::runtime_error ("shared memory segment could not be opened");
        }

        m_ptr = mmap ( NULL
                     , size()
                     , PROT_READ | PROT_WRITE
                     , MAP_SHARED
                     , fd
                     , 0
                     );
        if (m_ptr == (void*)-1)
        {
          err = errno;
          LOG(ERROR, "shared memory segment: " << name() << " could not be attached: " << strerror(err));
          ::close (fd);
          throw std::runtime_error ("shared memory segment could not be attached");
        }

        ::close (fd);
        DLOG(INFO, "shared memory segment " << name() << " created: " << m_ptr);
      }

      void segment_t::close ()
      {
        if (m_ptr)
        {
          munmap (m_ptr, size());
          if (! m_persistent)
          {
            shm_unlink (name().c_str());
          }
          m_ptr = 0;
        }
      }

      void segment_t::unlink ()
      {
        shm_unlink (name().c_str());
      }

      void *segment_t::ptr ()
      {
        return m_ptr;
      }

      void segment_t::assign_id (const type::segment_id_t id)
      {
        m_descriptor.id = id;
      }
    }
  }
}
