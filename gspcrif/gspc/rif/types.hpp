#ifndef GSPC_RIF_TYPES_HPP
#define GSPC_RIF_TYPES_HPP

#include <string>
#include <vector>
#include <map>
#include <list>
#include <boost/filesystem/path.hpp>

namespace gspc
{
  namespace rif
  {
    typedef std::string                        arg_t;
    typedef std::vector<arg_t>                 argv_t;
    typedef std::map<std::string, std::string> env_t;
    typedef int                                proc_t;
    typedef std::list<proc_t>                  proc_list_t;
    typedef std::list<boost::filesystem::path> search_path_t;
  }
}

#endif
