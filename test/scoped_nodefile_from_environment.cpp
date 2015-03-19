// alexander.petry@itwm.fraunhofer.de

#include <test/scoped_nodefile_from_environment.hpp>

#include <drts/drts.hpp>
#include <drts/private/option.hpp>

#include <util-generic/getenv.hpp>

#include <boost/format.hpp>
#include <boost/optional.hpp>

#include <fstream>
#include <stdexcept>

namespace test
{
  scoped_nodefile_from_environment::scoped_nodefile_from_environment
    ( boost::filesystem::path const& shared_directory
    , boost::program_options::variables_map& vm
    )
      : _temporary_file (shared_directory / boost::filesystem::unique_path())
  {
    boost::optional<const char*> const gspc_nodefile_for_tests
      (fhg::util::getenv ("GSPC_NODEFILE_FOR_TESTS"));
    if (!gspc_nodefile_for_tests)
    {
      throw std::runtime_error
        ("Environment variable GSPC_NODEFILE_FOR_TESTS is not set");
    }

    boost::filesystem::path const gspc_nodefile_for_tests_path
      (*gspc_nodefile_for_tests);

    if (!boost::filesystem::exists (gspc_nodefile_for_tests_path))
    {
      throw std::runtime_error
        (( boost::format ("Environment variable GSPC_NODEFILE_FOR_TESTS=\"%1%\" points to invalid location.")
         % gspc_nodefile_for_tests_path
         ).str()
        );
    }

    boost::filesystem::copy_file (gspc_nodefile_for_tests_path, _temporary_file);

    gspc::set_nodefile (vm, _temporary_file);
  }
}
