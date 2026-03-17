#include <fmt/core.h>
#include <fmt/std.h>
#include <gspc/util/read_file.hpp>

namespace gspc::util::error::read_file
{
  auto could_not_open
    ( std::filesystem::path const& path
    ) -> std::runtime_error
  {
    return std::runtime_error {fmt::format ("could not open {}", path)};
  }
}
