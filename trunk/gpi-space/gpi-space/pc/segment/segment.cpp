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
          LOG(WARN, "could not stop shared segment: " << m_name);
        }
      }

      segment_t::segment_t ( std::string const & name
                           , type::size_t sz
                           , type::segment_id_t id
                           , bool persistent
                           )
        : m_name (name)
        , m_id (id)
        , m_size (sz)
        , m_persistent (persistent)
        , m_ptr (0)
        , m_fd (-1)
      {
        if (m_name.empty())
          throw std::runtime_error ("invalid name argument to segment_t(): name must not be empty");
        if (m_name[0] != '/')
          m_name = "/" + m_name;
      }

      void segment_t::create ()
      {
        int err (0);
        err = shm_open (m_name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0);
        if (err < 0)
        {
          err = errno;
          LOG(ERROR, "shared memory segment: " << m_name << " could not be created: " << strerror(err));
          throw std::runtime_error ("shared memory segment could not be created");
        }
        else
        {
          m_fd = err;
        }
        err = ftruncate (m_fd, m_size);
        if (err < 0)
        {
          err = errno;
          LOG(ERROR, "shared memory segment: " << m_name << " could not be truncated: " << strerror(err));
          throw std::runtime_error ("shared memory segment could not be truncated");
        }

        m_ptr = mmap ( NULL
                     , m_size
                     , PROT_READ | PROT_WRITE
                     , MAP_SHARED
                     , m_fd
                     , 0
                     );
        if (m_ptr == (void*)-1)
        {
          err = errno;
          LOG(ERROR, "shared memory segment: " << m_name << " could not be attached: " << strerror(err));
          throw std::runtime_error ("shared memory segment could not be attached");
        }

        LOG(INFO, "shared memory segment " << m_name << " created: " << m_ptr);
      }

      void segment_t::open ()
      {
        int err (0);
        err = shm_open (m_name.c_str(), O_RDWR, 0);
        if (err < 0)
        {
          err = errno;
          LOG(ERROR, "shared memory segment: " << m_name << " could not be opened: " << strerror(err));
          throw std::runtime_error ("shared memory segment could not be opened");
        }
        else
        {
          m_fd = err;
        }

        m_ptr = mmap ( NULL
                     , m_size
                     , PROT_READ | PROT_WRITE
                     , MAP_SHARED
                     , m_fd
                     , 0
                     );
        if (m_ptr == (void*)-1)
        {
          err = errno;
          LOG(ERROR, "shared memory segment: " << m_name << " could not be attached: " << strerror(err));
          throw std::runtime_error ("shared memory segment could not be attached");
        }

        LOG(INFO, "shared memory segment " << m_name << " created: " << m_ptr);
      }

      void segment_t::close ()
      {
        if (m_ptr)
        {
          close (m_fd);
          munmap (m_ptr, m_size);
          if (! m_persistent)
          {
            shm_unlink (m_name.c_str());
          }
        }
      }

      void *segment_t::ptr ()
      {
        return m_ptr;
      }

      void segment_t::assign_id (const type::segment_id_t id)
      {
        m_id = id;
      }
    }
  }
}
