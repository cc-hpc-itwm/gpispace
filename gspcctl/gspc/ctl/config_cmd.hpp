#ifndef GSPC_CTL_CONFIG_CMD_HPP
#define GSPC_CTL_CONFIG_CMD_HPP

#include <string>
#include <vector>
#include <iostream>

namespace gspc
{
  namespace ctl
  {
    namespace cmd
    {
      int config_cmd ( std::vector<std::string> const &argv
                     , std::istream &inp = std::cin
                     , std::ostream &out = std::cout
                     , std::ostream &err = std::cerr
                     );
    }
  }
}

#endif
