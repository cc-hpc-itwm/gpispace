#include <gspc/installation_path.hpp>

#include <drts/drts.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/test/unit_test.hpp>

#include <stdexcept>

BOOST_AUTO_TEST_CASE (our_installation_does_not_throw)
{
  BOOST_CHECK_NO_THROW (gspc::installation_path{});
}

BOOST_AUTO_TEST_CASE (wrong_installation_throws)
{
  fhg::util::temporary_path const temporary_path;
  boost::filesystem::path const path (temporary_path);

  boost::program_options::variables_map vm;
  gspc::set_gspc_home (vm, path);

  fhg::util::testing::require_exception
    ( [&]
      {
        gspc::installation const installation (vm);
      }
    , std::logic_error
        ( "given gspc home that is not the same as the installation "
          "libgspc.so is loaded from"
        )
    );

  fhg::util::testing::require_exception
    ( [&]
      {
        gspc::installation const installation (path);
      }
    , std::logic_error
        ( "given gspc home that is not the same as the installation "
          "libgspc.so is loaded from"
        )
    );
}
