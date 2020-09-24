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
                           )
        : m_ptr (nullptr)
        , _name (name)
        , _size (sz)
      {
        if (name.empty())
          throw std::runtime_error ("invalid name argument to segment_t(): name must not be empty");

        if (_name[0] != '/')
        {
          _name = '/' + _name;
        }
      }

      void segment_t::create (const mode_t mode)
      {
        int fd (-1);

        try
        {
          fd = fhg::util::syscall::shm_open (_name.c_str(), O_RDWR | O_CREAT | O_EXCL, mode);
        }
        catch (boost::system::system_error const&)
        {
          std::throw_with_nested
            ( std::runtime_error
                ("shared memory segment '" + _name + "' could not be created")
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
          fhg::util::syscall::ftruncate (fd, _size);
        }
        catch (boost::system::system_error const&)
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
        catch (boost::system::system_error const&)
        {
          std::throw_with_nested
            ( std::runtime_error
                ("shared memory segment '" + _name + "' could not be attached")
            );
        }
      }

      void segment_t::open ()
      {
        int fd (-1);

        try
        {
          fd = fhg::util::syscall::shm_open (_name.c_str(), O_RDWR, 0);
        }
        catch (boost::system::system_error const& se)
        {
          std::throw_with_nested
            ( std::runtime_error
                ("shared memory segment '" + _name + "' could not be opened")
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
                                           , _size
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
                ("shared memory segment '" + _name + "' could not be attached")
            );
        }
      }

      void segment_t::close ()
      {
        if (m_ptr)
        {
          fhg::util::syscall::munmap (m_ptr, _size);
          m_ptr = nullptr;
        }
      }

      void segment_t::unlink ()
      {
        fhg::util::syscall::shm_unlink(_name.c_str());
      }

      void *segment_t::ptr ()
      {
        return m_ptr;
      }

      const void *segment_t::ptr () const
      {
        return m_ptr;
      }
    }
  }
}
