
#define BOOST_TEST_MODULE drts_installation
#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>

#include <fhg/util/temporary_path.hpp>
#include <fhg/util/boost/test/require_exception.hpp>
#include <fhg/revision.hpp>

#include <boost/program_options.hpp>

#include <fstream>

BOOST_AUTO_TEST_CASE (installation_set_gspc_home_to_directory_without_revision)
{
  fhg::util::temporary_path const temporary_path;
  boost::filesystem::path const path (temporary_path);

  fhg::util::boost::test::require_exception<std::invalid_argument>
    ([&path]()
    {
      boost::program_options::variables_map vm;
      gspc::set_gspc_home (vm, path);
      gspc::installation const installation (vm);
    }
    , ( boost::format ("GSPC revision mismatch: File '%1%' does not exist.")
      % (boost::filesystem::canonical (path) / "revision")
      ).str()
    );
}

BOOST_AUTO_TEST_CASE (installation_set_gspc_home_to_directory_with_bad_revision)
{
  fhg::util::temporary_path const temporary_path;
  boost::filesystem::path const path (temporary_path);

  std::ofstream ((path / "revision").string()) << "-";

  fhg::util::boost::test::require_exception<std::invalid_argument>
    ([&path]()
    {
      boost::program_options::variables_map vm;
      gspc::set_gspc_home (vm, path);
      gspc::installation const installation (vm);
    }
    , ( boost::format ( "GSPC revision mismatch: Expected '%1%'"
                      ", installation in '%2%' has version '%3%'"
                      )
      % fhg::project_revision()
      % boost::filesystem::canonical (path)
      % "-"
      ).str()
    );
}