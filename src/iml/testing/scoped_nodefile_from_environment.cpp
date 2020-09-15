#include <iml/testing/scoped_nodefile_from_environment.hpp>

#include <iml/client/iml.hpp>
#include <iml/client/private/option.hpp>

#include <util-generic/getenv.hpp>

#include <boost/format.hpp>
#include <boost/optional.hpp>

#include <fstream>
#include <stdexcept>

namespace iml_test
{
  scoped_nodefile_from_environment::scoped_nodefile_from_environment
    ( boost::filesystem::path const& shared_directory
    , boost::program_options::variables_map& vm
    )
      : _temporary_file (shared_directory / boost::filesystem::unique_path())
  {
    boost::optional<const char*> iml_nodefile_for_tests
      (fhg::util::getenv ("IML_NODEFILE_FOR_TESTS"));
    if (!iml_nodefile_for_tests)
    {
      // \todo Remove automatic fallback once standalone. Convenience
      // for transition period only. Once moved out this automatic
      // setting should be done on the GPISpace-side test level: A
      // test that uses IML shall set that variable.
      boost::optional<const char*> const gspc_nodefile_for_tests
        (fhg::util::getenv ("GSPC_NODEFILE_FOR_TESTS"));
      if (!gspc_nodefile_for_tests)
      {
        throw std::runtime_error
          ("Environment variable IML_NODEFILE_FOR_TESTS is not set");
      }
      iml_nodefile_for_tests = gspc_nodefile_for_tests;
    }

    boost::filesystem::path const iml_nodefile_for_tests_path
      (*iml_nodefile_for_tests);

    if (!boost::filesystem::exists (iml_nodefile_for_tests_path))
    {
      throw std::runtime_error
        (( boost::format ("Environment variable IML_NODEFILE_FOR_TESTS=\"%1%\" points to invalid location.")
         % iml_nodefile_for_tests_path
         ).str()
        );
    }

    boost::filesystem::copy_file (iml_nodefile_for_tests_path, _temporary_file);

    iml_client::set_nodefile (vm, _temporary_file);
  }
}
