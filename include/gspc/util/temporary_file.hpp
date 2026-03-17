#pragma once

#include <gspc/detail/export.hpp>

#include <filesystem>
#include <stdexcept>


  namespace gspc::util
  {
    namespace error::temporary_file
    {
      auto GSPC_EXPORT already_exists
        ( std::filesystem::path const&
        ) -> std::logic_error
        ;
    }

    class temporary_file
    {
    public:
      temporary_file (std::filesystem::path const& path)
        : _path (std::filesystem::absolute (path))
      {
        if (std::filesystem::exists (_path))
        {
          throw error::temporary_file::already_exists (path);
        }
      }

      ~temporary_file()
      {
        std::filesystem::remove (_path);
      }

      temporary_file (temporary_file const&) = delete;
      temporary_file& operator= (temporary_file const&) = delete;
      temporary_file (temporary_file&&) = delete;
      temporary_file& operator= (temporary_file&&) = delete;

      operator std::filesystem::path() const
      {
        return _path;
      }

    private:
      std::filesystem::path _path;
    };
  }
