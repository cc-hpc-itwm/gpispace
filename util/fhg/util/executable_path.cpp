// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/util/executable_path.hpp>

#include <util-generic/syscall.hpp>

#include <boost/filesystem/operations.hpp>

namespace fhg
{
  namespace util
  {
    boost::filesystem::path executable_path()
    {
      //! \todo Other systems. See
      //! http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe

      return boost::filesystem::canonical ( boost::filesystem::path ("/")
                                          / "proc"
                                          / std::to_string (syscall::getpid())
                                          / "exe"
                                          );
    }
  }
}
