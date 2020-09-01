#include <test/virtual_memory_socket_name_for_localhost.hpp>

#include <drts/private/option.hpp>

namespace test
{
  void set_virtual_memory_socket_name_for_localhost
    (boost::program_options::variables_map& vm)
  {
    gspc::set_virtual_memory_socket ( vm
                                    , boost::filesystem::temp_directory_path()
                                    / boost::filesystem::unique_path()
                                    );
  }
}
