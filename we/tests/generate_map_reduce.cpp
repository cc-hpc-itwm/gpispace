#include <sstream>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/program_options.hpp>

#include <we/we.hpp>
#include "map_reduce.hpp"

namespace po = boost::program_options;

int main (int argc, char ** argv)
{
  po::options_description desc("options");

  desc.add_options()
    ("help,h", "this message")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cerr << desc << std::endl;
    return EXIT_SUCCESS;
  }

  we::util::codec::encode
    ( std::cout
    , we::activity_t(we::tests::map_reduce<we::activity_t>::generate())
    );

  return EXIT_SUCCESS;
}

