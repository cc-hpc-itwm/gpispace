// mirko.rahn@itwm.fraunhofer.de

#include <test/scoped_state_directory.hpp>

#include <drts/drts.hpp>

#include <boost/filesystem.hpp>

namespace test
{
  scoped_state_directory::scoped_state_directory
    (boost::program_options::variables_map& vm)
      : _temporary_path ( boost::filesystem::temp_directory_path()
                        / boost::filesystem::unique_path()
                        )
  {
    boost::filesystem::create_directories (_temporary_path);

    gspc::set_state_directory (vm, _temporary_path);
  }
}
