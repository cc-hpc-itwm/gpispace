// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_PIDFILE_WRITER_HPP
#define FHG_UTIL_PIDFILE_WRITER_HPP

#include <string>

namespace fhg
{
  namespace util
  {
    struct pidfile_writer
    {
      pidfile_writer (std::string filename);

      //! \note Can't be in dtor, as we don't want to write the
      //! pidfile if something in between (daemonizing) fails.
      void write() const;

    private:
      int _fd;
    };
  }
}

#endif
