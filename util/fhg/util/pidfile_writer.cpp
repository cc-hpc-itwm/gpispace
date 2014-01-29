// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/util/pidfile_writer.hpp>

#include <stdexcept>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace fhg
{
  namespace util
  {
    pidfile_writer::pidfile_writer (std::string filename)
      : _fd (open (filename.c_str(), O_CREAT|O_RDWR, 0640))
    {
      if (_fd < 0)
      {
        throw std::runtime_error ("could not open pidfile for writing: " + std::string (strerror (errno)));
      }
    }

    void pidfile_writer::write() const
    {
      if (lockf (_fd, F_TLOCK, 0) < 0)
      {
        throw std::runtime_error ("could not lock pidfile: " + std::string (strerror (errno)));
      }

      if (ftruncate (_fd, 0) < 0)
      {
        throw std::runtime_error ("could not truncate pidfile: " + std::string (strerror (errno)));
      }

      char buf[32];
      if (snprintf (buf, sizeof (buf), "%d\n", getpid()) < 0)
      {
        throw std::runtime_error ("could not sprintf for pidfile: " + std::string (strerror (errno)));
      }

      if (::write (_fd, buf, strlen (buf)) != (int)strlen (buf))
      {
        throw std::runtime_error ("could not write to pidfile: " + std::string (strerror (errno)));
      }

      if (fsync (_fd) < 0)
      {
        throw std::runtime_error ("could not fsync pidfile: " + std::string (strerror (errno)));
      }
    }
  }
}
