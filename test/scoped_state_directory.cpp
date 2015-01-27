// mirko.rahn@itwm.fraunhofer.de

#include <test/scoped_state_directory.hpp>

#include <drts/drts.hpp>
#include <drts/private/option.hpp>

namespace test
{
  scoped_state_directory::scoped_state_directory
    ( boost::filesystem::path const& shared_directory
    , boost::program_options::variables_map& vm
    )
      : _temporary_path (shared_directory / boost::filesystem::unique_path())
  {
    boost::filesystem::create_directories (_temporary_path);

    gspc::set_state_directory (vm, _temporary_path);
  }
}
