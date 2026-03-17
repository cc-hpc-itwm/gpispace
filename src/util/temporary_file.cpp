#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <fmt/core.h>
#include <gspc/util/temporary_file.hpp>

namespace gspc::util::error::temporary_file
{
  auto already_exists
    ( std::filesystem::path const& path
    ) -> std::logic_error
  {
    return std::logic_error
      { fmt::format ("Temporary file {} already exists.", path)
      };
  }
}
