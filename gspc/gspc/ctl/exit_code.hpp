#ifndef GSPC_CTL_EXIT_CODE_HPP
#define GSPC_CTL_EXIT_CODE_HPP

#include <string>
#include <sysexits.h>

namespace gspc
{
  namespace ctl
  {
    std::string strerror (int ec);
  }
}

#endif
