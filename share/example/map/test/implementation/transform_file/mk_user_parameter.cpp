// mirko.rahn@itwm.fraunhofer.de

#include <map/test/implementation/transform_file/type.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <iostream>

int main (int argc, char** argv)
{
  boost::program_options::options_description options_description;

  options_description.add_options()
    ( "input"
    , boost::program_options::value<boost::filesystem::path>()->required()
    , "input file name"
    )
    ( "output"
    , boost::program_options::value<boost::filesystem::path>()->required()
    , "output file name"
    )
    ( "size"
    , boost::program_options::value<unsigned long>()->required()
    , "size"
    )
    ;

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser (argc, argv)
    . options (options_description).run()
    , vm
    );
  vm.notify();

  std::cout << transform_file::to_bytearray
    ( transform_file::parameter
      ( vm["input"].as<boost::filesystem::path>()
      , vm["output"].as<boost::filesystem::path>()
      , vm["size"].as<unsigned long>()
      )
    );

  return 0;
}
