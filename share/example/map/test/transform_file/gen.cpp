// mirko.rahn@itwm.fraunhofer.de

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <algorithm>
#include <fstream>
#include <random>

int main (int argc, char** argv)
{
  boost::program_options::options_description options_description;

  options_description.add_options()
    ( "data"
    , boost::program_options::value<boost::filesystem::path>()->required()
    , "data file name"
    )
    ( "verify"
    , boost::program_options::value<boost::filesystem::path>()->required()
    , "verify data file name"
    )
    ( "size"
    , boost::program_options::value<unsigned long>()->required()
    , "size of data file (also: size of verify file)"
    )
    ( "chunk-size"
    , boost::program_options::value<unsigned long>()
      ->default_value (2UL << 20UL)
    , "chunk size"
    )
    ;

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser (argc, argv)
    . options (options_description).run()
    , vm
    );
  vm.notify();

  boost::filesystem::path const data_file
    (vm["data"].as<boost::filesystem::path>());
  boost::filesystem::path const verify_file
    (vm["verify"].as<boost::filesystem::path>());
  unsigned long const size (vm["size"].as<unsigned long>());
  unsigned long const chunk_size (vm["chunk-size"].as<unsigned long>());

  std::ofstream data (data_file.string(), std::ostream::binary);

  if (!data)
  {
    std::runtime_error
      ((boost::format ("Could not open '%1%'") % data_file).str());
  }

  std::ofstream verify (verify_file.string(), std::ostream::binary);

  if (!verify)
  {
    std::runtime_error
      ((boost::format ("Could not open '%1%'") % verify_file).str());
  }

  std::mt19937 generator;
  std::uniform_int_distribution<> number (0, 255);

  std::vector<char> chunk (chunk_size);

  unsigned long bytes_left (size);

  while (bytes_left)
  {
    unsigned long const bytes (std::min (chunk_size, bytes_left));

    for (unsigned long i (0); i < bytes; ++i)
    {
      chunk[i] = number (generator);
    }

    data << std::string (chunk.data(), chunk.data() + bytes);

    std::transform ( chunk.data(), chunk.data() + bytes
                   , chunk.data()
                   , [](char c) { return std::tolower (c); }
                   );

    verify << std::string (chunk.data(), chunk.data() + bytes);

    bytes_left -= bytes;
  }

  return 0;
}
