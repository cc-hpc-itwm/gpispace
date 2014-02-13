#include "segment.hpp"

// needs linking with -lrt
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <fhglog/LogMacros.hpp>
#include <fhg/assert.hpp>
#include <fhg/syscall.hpp>

#include <gpi-space/pc/type/flags.hpp>

namespace gpi
{
  namespace pc
  {
    namespace segment
    {
      segment_t::~segment_t()
      {
          close ();
      }

      segment_t::segment_t ( std::string const & nme
                           , const type::size_t sz
                           , const type::segment_id_t id
                           )
        : m_ptr (0)
      {
        if (nme.empty())
          throw std::runtime_error ("invalid name argument to segment_t(): name must not be empty");

        if (nme[0] != '/')
          m_descriptor.name = "/" + nme;
        else
          m_descriptor.name = nme;
        m_descriptor.local_size = sz;
        m_descriptor.avail = sz;
        m_descriptor.id = id;
      }

      bool segment_t::is_special () const
      {
        return gpi::flag::is_set( m_descriptor.flags
                                , gpi::pc::F_SPECIAL
                                );
      }

      void segment_t::create (const mode_t mode)
      {
        assert (! is_special ());

        int fd (-1);

        try
        {
          fd = fhg::syscall::shm_open (name().c_str(), O_RDWR | O_CREAT | O_EXCL, mode);
        }
        catch (boost::system::system_error const& se)
        {
          LOG(ERROR, "shared memory segment: " << name() << " could not be created: " << se.what());
          throw std::runtime_error ("shared memory segment could not be created");
        }

        try
        {
          fhg::syscall::ftruncate (fd, size());
        }
        catch (boost::system::system_error const& se)
        {
          LOG( ERROR
             , "shared memory segment: " << name()
             << " could not be truncated to size: " << size()
             << ": " << se.what()
             );
          fhg::syscall::close (fd);
          throw std::runtime_error
            ("shared memory segment could not be truncated");
        }

        try
        {
          m_ptr = fhg::syscall::mmap ( NULL
                                     , size()
                                     , PROT_READ | PROT_WRITE
                                     , MAP_SHARED
                                     , fd
                                     , 0
                                     );
        }
        catch (boost::system::system_error const& se)
        {
          LOG(ERROR, "shared memory segment: " << name() << " could not be attached: " << se.what());
          fhg::syscall::close (fd);
          throw std::runtime_error ("shared memory segment could not be attached");
        }

        fhg::syscall::close (fd);
      }

      void segment_t::open ()
      {
        assert (! is_special ());

        int fd (-1);

        try
        {
          fd = fhg::syscall::shm_open (name().c_str(), O_RDWR, 0);
        }
        catch (boost::system::system_error const& se)
        {
          LOG(ERROR, "shared memory segment: " << name() << " could not be opened: " << se.what());
          throw std::runtime_error ("shared memory segment could not be opened");
        }

        try
        {
          m_ptr = fhg::syscall::mmap ( NULL
                                     , size()
                                     , PROT_READ | PROT_WRITE
                                     , MAP_SHARED
                                     , fd
                                     , 0
                                     );
        }
        catch (boost::system::system_error const& se)
        {
          LOG(ERROR, "shared memory segment: " << name() << " could not be attached: " << se.what());
          fhg::syscall::close (fd);
          throw std::runtime_error ("shared memory segment could not be attached");
        }

        fhg::syscall::close (fd);
      }

      void segment_t::close ()
      {
        if (is_special())
          return;

        if (m_ptr)
        {
          fhg::syscall::munmap (m_ptr, size());
          m_ptr = 0;
        }
      }

      void segment_t::unlink ()
      {
        assert (! is_special ());
        fhg::syscall::shm_unlink(name().c_str());
      }

      void *segment_t::ptr ()
      {
        return m_ptr;
      }

      const void *segment_t::ptr () const
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
