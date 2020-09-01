#include <iml/vmem/netdev_id.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

namespace iml_client
{
#define GET(_name, _type)                               \
  boost::optional<_type> get_ ## _name                  \
    (boost::program_options::variables_map const&)
#define REQUIRE(_name, _type)                                           \
  _type require_ ## _name (boost::program_options::variables_map const&)
#define ACCESS(_name, _type)                    \
  GET (_name, _type);                           \
  REQUIRE (_name, _type)

  ACCESS (iml_home, boost::filesystem::path);
  ACCESS (nodefile, boost::filesystem::path);

  ACCESS (virtual_memory_socket, boost::filesystem::path);
  ACCESS (virtual_memory_port, unsigned short);
  ACCESS (virtual_memory_startup_timeout, unsigned long);
  ACCESS (virtual_memory_netdev_id, fhg::iml::vmem::netdev_id);

  ACCESS (rif_entry_points_file, boost::filesystem::path);
  ACCESS (rif_port, unsigned short);
  ACCESS (rif_strategy, std::string);

  ACCESS (rif_strategy_parameters, std::vector<std::string>);
  char const* name_rif_strategy_parameters();

#undef ACCESS
#undef REQUIRE
#undef GET
}
