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

#include <iml/vmem/gaspi/pc/type/flags.hpp>

#include <boost/system/system_error.hpp>

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

      segment_t::segment_t ( std::string const & name
                           , const type::size_t sz
                           , const type::segment_id_t id
                           )
        : m_ptr (nullptr)
      {
        if (name.empty())
          throw std::runtime_error ("invalid name argument to segment_t(): name must not be empty");

        if (name[0] != '/')
          m_descriptor.name = "/" + name;
        else
          m_descriptor.name = name;
        m_descriptor.local_size = sz;
        m_descriptor.avail = sz;
        m_descriptor.id = id;
      }

      void segment_t::create (const mode_t mode)
      {
        int fd (-1);

        try
        {
          fd = fhg::util::syscall::shm_open (name().c_str(), O_RDWR | O_CREAT | O_EXCL, mode);
        }
        catch (boost::system::system_error const&)
        {
          std::throw_with_nested
            ( std::runtime_error
                ("shared memory segment '" + name() + "' could not be created")
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
          fhg::util::syscall::ftruncate (fd, size());
        }
        catch (boost::system::system_error const&)
        {
          std::throw_with_nested
            ( std::runtime_error
                ( "shared memory segment '" + name()
                + "' could not be truncated to size " + std::to_string (size())
                )
            );
        }

        try
        {
          m_ptr = fhg::util::syscall::mmap ( nullptr
                                           , size()
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
                ("shared memory segment '" + name() + "' could not be attached")
            );
        }
      }

      void segment_t::open ()
      {
        int fd (-1);

        try
        {
          fd = fhg::util::syscall::shm_open (name().c_str(), O_RDWR, 0);
        }
        catch (boost::system::system_error const& se)
        {
          std::throw_with_nested
            ( std::runtime_error
                ("shared memory segment '" + name() + "' could not be opened")
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
                                           , size()
                                           , PROT_READ | PROT_WRITE
                                           , MAP_SHARED
                                           , fd
                                           , 0
                                           );
        }
        catch (boost::system::system_error const& se)
        {
          std::throw_with_nested
            ( std::runtime_error
                ("shared memory segment '" + name() + "' could not be attached")
            );
        }
      }

      void segment_t::close ()
      {
        if (m_ptr)
        {
          fhg::util::syscall::munmap (m_ptr, size());
          m_ptr = nullptr;
        }
      }

      void segment_t::unlink ()
      {
        fhg::util::syscall::shm_unlink(name().c_str());
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