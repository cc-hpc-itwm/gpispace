#pragma once

#include <gspc/detail/export.hpp>

#include <filesystem>
#include <stdexcept>
#include <utility>


  namespace gspc::util
  {
    namespace error
    {
      struct GSPC_EXPORT path_already_exists : std::logic_error
      {
        std::filesystem::path path;
        path_already_exists (std::filesystem::path);
      };
    }

    class temporary_path
    {
    public:
      temporary_path (std::filesystem::path const& path)
        : _path (std::filesystem::absolute (path))
      {
        if (std::filesystem::exists (_path))
        {
          throw error::path_already_exists (path);
        }

        std::filesystem::create_directories (_path);
      }
      ~temporary_path()
      {
        std::filesystem::remove_all (_path);
      }
      operator std::filesystem::path() const
      {
        return _path;
      }

      temporary_path (temporary_path const&) = delete;
      temporary_path& operator= (temporary_path const&) = delete;
      temporary_path (temporary_path&&) = delete;
      temporary_path& operator= (temporary_path&&) = delete;

    private:
      std::filesystem::path _path;
    };
  }
