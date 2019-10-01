#include <util-generic/finally.hpp>
#include <util-generic/syscall.hpp>

#include <boost/format.hpp>

#include <string>

namespace
{
  bool is_using_port (std::string const& component, unsigned int port)
  {
    unsigned int const size (30);
    char data[size] = {0};

    FILE* pf
      ( fhg::util::syscall::popen
          ( ( boost::format
               ("netstat -lntp | grep -w '%1%' | awk '{print $NF}' | cut -d'/' -f2")
            % port
            ).str().c_str()
          , "r"
          )
      );

    FHG_UTIL_FINALLY ([&] { fhg::util::syscall::pclose (pf); });
    fhg::util::syscall::fgets (data, size, pf);

    std::string str (data);
    str.erase (str.find_last_not_of ("\n") + 1);

    return (str == component);
  }
}
