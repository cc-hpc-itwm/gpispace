// mirko.rahn@itwm.fraunhofer.de

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

namespace gspc
{
#define SET(_name, _type) \
  void set_ ## _name (boost::program_options::variables_map&, _type const&)
#define GET(_name, _type)                               \
  boost::optional<_type> get_ ## _name                  \
    (boost::program_options::variables_map const&)
#define REQUIRE(_name, _type)                                           \
  _type require_ ## _name (boost::program_options::variables_map const&)
#define ACCESS(_name, _type)                    \
  SET (_name, _type);                           \
  GET (_name, _type);                           \
  REQUIRE (_name, _type);

  ACCESS (log_host, std::string);
  ACCESS (log_port, unsigned short);
  ACCESS (log_level, std::string);
  ACCESS (gui_host, std::string);
  ACCESS (gui_port, unsigned short);

  ACCESS (state_directory, boost::filesystem::path);
  ACCESS (gspc_home, boost::filesystem::path);
  ACCESS (nodefile, boost::filesystem::path);
  GET (application_search_path, boost::filesystem::path);
  REQUIRE (application_search_path, boost::filesystem::path);

  ACCESS (virtual_memory_per_node, unsigned long);
  ACCESS (virtual_memory_socket, boost::filesystem::path);
  ACCESS (virtual_memory_port, unsigned short);
  ACCESS (virtual_memory_startup_timeout, unsigned long);

#undef ACCESS
#undef REQUIRE
#undef GET
#undef SET
}
