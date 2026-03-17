#pragma once

#include <gspc/detail/export.hpp>
#include <boost/lexical_cast.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>


  namespace gspc::util
  {
    namespace error::read_file
    {
      auto GSPC_EXPORT could_not_open
        ( std::filesystem::path const&
        ) -> std::runtime_error
        ;
    }

    template<typename T = std::string>
      T read_file (std::filesystem::path const& path)
    {
      std::ifstream ifs (path, std::ifstream::binary);

      if (not ifs)
      {
        throw error::read_file::could_not_open (path);
      }

      ifs >> std::noskipws;

      return {std::istream_iterator<char> (ifs), std::istream_iterator<char>()};
    }

    template<typename T>
      T read_file_as (std::filesystem::path const& path)
    {
      return ::boost::lexical_cast<T> (read_file (path));
    }
  }
