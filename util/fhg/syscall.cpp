// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/syscall.hpp>

#include <boost/system/system_error.hpp>

#include <sys/socket.h>

namespace fhg
{
  namespace syscall
  {
    namespace
    {
      template<typename T>
        T negative_one_fails_with_errno (int rc)
      {
        if (rc == -1)
        {
          throw boost::system::system_error
            (boost::system::error_code (errno, boost::system::system_category()));
        }
        return T (rc);
      }

      template<>
        void negative_one_fails_with_errno<void> (int rc)
      {
        if (rc == -1)
        {
          throw boost::system::system_error
            (boost::system::error_code (errno, boost::system::system_category()));
        }
      }
    }

    void chdir (const char* path)
    {
      return negative_one_fails_with_errno<void> (::chdir (path));
    }

    void close (int fd)
    {
      return negative_one_fails_with_errno<void> (::close (fd));
    }

    pid_t fork()
    {
      return negative_one_fails_with_errno<pid_t> (::fork());
    }

    pid_t setsid()
    {
      return negative_one_fails_with_errno<pid_t> (::setsid());
    }

    void shutdown (int sockfd, int how)
    {
      return negative_one_fails_with_errno<void> (::shutdown (sockfd, how));
    }
  }
}
