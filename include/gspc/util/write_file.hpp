#pragma once

#include <gspc/detail/export.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>


  namespace gspc::util
  {
    namespace error::write_file
    {
      auto GSPC_EXPORT could_not_open
        ( std::filesystem::path const&
        ) -> std::runtime_error
        ;
      auto GSPC_EXPORT could_not_write
        ( std::filesystem::path const&
        ) -> std::runtime_error
        ;
    }

    template<typename T>
      void write_file (std::filesystem::path const& path, T const& x)
    {
      auto stream {std::ofstream {path}};

      if (!stream)
      {
        throw error::write_file::could_not_open (path);
      }

      stream << x;

      if (!stream)
      {
        throw error::write_file::could_not_write (path);
      }
    }
  }
