#include <gspc/util/filesystem_lock_directory.hpp>

#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <exception>
#include <fmt/core.h>
#include <stdexcept>


  namespace gspc::util
  {
    failed_to_create_lock::failed_to_create_lock
      (std::filesystem::path const& path)
        : std::runtime_error
          { fmt::format ("Failed to create lock for {}.", path)
          }
        , _path (path)
    {}

    filesystem_lock_directory::filesystem_lock_directory
      (std::filesystem::path const& path)
    try
      : temporary_path {path}
    {}
    catch (...)
    {
      std::throw_with_nested (failed_to_create_lock (path));
    }
  }
