#include <boost/test/unit_test.hpp>

#include <iml/revision.hpp>

#include <iml/client/iml.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/program_options.hpp>

#include <fstream>

BOOST_AUTO_TEST_CASE (installation_set_iml_home_to_directory_without_revision)
{
  fhg::util::temporary_path const temporary_path;
  boost::filesystem::path const path (temporary_path);

  fhg::util::testing::require_exception
    ([&path]()
    {
      boost::program_options::variables_map vm;
      iml_client::set_iml_home (vm, path);
      iml_client::installation const installation (vm);
    }
    , std::invalid_argument
        ( ( boost::format ("IML revision mismatch: File '%1%' does not exist.")
          % (boost::filesystem::canonical (path) / "revision")
          ).str()
        )
    );
}

BOOST_AUTO_TEST_CASE (installation_set_iml_home_to_directory_with_bad_revision)
{
  fhg::util::temporary_path const temporary_path;
  boost::filesystem::path const path (temporary_path);

  std::ofstream ((path / "revision").string()) << "-";

  fhg::util::testing::require_exception
    ([&path]()
    {
      boost::program_options::variables_map vm;
      iml_client::set_iml_home (vm, path);
      iml_client::installation const installation (vm);
    }
    , std::invalid_argument
        ( ( boost::format ( "IML revision mismatch: Expected '%1%'"
                            ", installation in '%2%' has version '%3%'"
                          )
          % fhg::iml::project_revision()
          % boost::filesystem::canonical (path)
          % "-"
          ).str()
        )
    );
}
