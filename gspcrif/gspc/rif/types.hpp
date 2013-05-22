#ifndef GSPC_RIF_TYPES_HPP
#define GSPC_RIF_TYPES_HPP

#include <string>
#include <vector>
#include <map>

namespace gspc
{
  namespace rif
  {
    typedef std::string                        arg_t;
    typedef std::vector<arg_t>                 argv_t;
    typedef std::map<std::string, std::string> env_t;
  }
}

#endif
