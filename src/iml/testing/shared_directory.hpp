#pragma once

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace iml_test
{
  namespace options
  {
    boost::program_options::options_description shared_directory();
  }

  boost::filesystem::path shared_directory
    (boost::program_options::variables_map const&);
}
