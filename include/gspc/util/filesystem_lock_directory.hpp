#pragma once

#include <gspc/util/temporary_path.hpp>

#include <filesystem>

#include <stdexcept>


  namespace gspc::util
  {
    class failed_to_create_lock : public std::runtime_error
    {
    public:
      failed_to_create_lock (std::filesystem::path const&);

      std::filesystem::path const& path() const noexcept
      {
        return _path;
      }

    private:
      std::filesystem::path const _path;
    };

    struct filesystem_lock_directory : public temporary_path
    {
      filesystem_lock_directory (std::filesystem::path const&);
    };
  }
