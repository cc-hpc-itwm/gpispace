// mirko.rahn@itwm.fraunhofer.de

#include <test/scoped_nodefile_with_localhost.hpp>

#include <drts/drts.hpp>

#include <fhg/util/hostname.hpp>

#include <boost/format.hpp>

#include <fstream>
#include <stdexcept>

namespace test
{
  scoped_nodefile_with_localhost::scoped_nodefile_with_localhost
    (boost::program_options::variables_map& vm)
      : _temporary_file ( boost::filesystem::temp_directory_path()
                        / boost::filesystem::unique_path()
                        )
  {
    boost::filesystem::path const path (_temporary_file);

    std::ofstream stream (path.string());

    if (!stream)
    {
      throw std::runtime_error
        ((boost::format ("Could not open '%1%' for writing") % path).str());
    }

    stream << fhg::util::hostname() << std::endl;

    gspc::set_nodefile (vm, path);
  }
}
