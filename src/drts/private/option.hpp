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
  REQUIRE (_name, _type)

  ACCESS (log_host, std::string);
  ACCESS (log_port, unsigned short);
  ACCESS (log_level, std::string);

  ACCESS (log_directory, boost::filesystem::path);
  GET (gspc_home, boost::filesystem::path);
  REQUIRE (gspc_home, boost::filesystem::path);
  ACCESS (nodefile, boost::filesystem::path);
  GET (application_search_path, boost::filesystem::path);
  REQUIRE (application_search_path, boost::filesystem::path);

  ACCESS (virtual_memory_socket, boost::filesystem::path);
  ACCESS (virtual_memory_port, unsigned short);
  ACCESS (virtual_memory_startup_timeout, unsigned long);

  ACCESS (rif_entry_points_file, boost::filesystem::path);
  ACCESS (rif_port, unsigned short);
  ACCESS (rif_strategy, std::string);

  GET (rif_strategy_parameters, std::vector<std::string>);
  REQUIRE (rif_strategy_parameters, std::vector<std::string>);
  char const* name_rif_strategy_parameters();

#undef ACCESS
#undef REQUIRE
#undef GET
#undef SET
}
