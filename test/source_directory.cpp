// mirko.rahn@itwm.fraunhofer.de

#include <test/source_directory.hpp>

#include <fhg/util/boost/program_options/validators/existing_directory.hpp>

namespace test
{
  namespace validators = fhg::util::boost::program_options;

  namespace options
  {
    boost::program_options::options_description source_directory()
    {
      boost::program_options::options_description source_directory;

      source_directory.add_options()
        ( name::source_directory
        , boost::program_options::value<validators::existing_directory>()
        ->required()
        , "source directory"
        )
        ;

      return source_directory;
    }
  }

  boost::filesystem::path source_directory
    (boost::program_options::variables_map const& vm)
  {
    return vm[options::name::source_directory]
      .as<validators::existing_directory>();
  }
}
