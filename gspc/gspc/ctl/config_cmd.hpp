#ifndef GSPC_CTL_CONFIG_CMD_HPP
#define GSPC_CTL_CONFIG_CMD_HPP

#include <string>
#include <vector>

namespace gspc
{
  namespace ctl
  {
    namespace cmd
    {
      int config_cmd (std::vector<std::string> const& argv);
    }
  }
}

#endif
