#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <fmt/core.h>
#include <gspc/util/temporary_path.hpp>
#include <utility>

namespace gspc::util::error
{
  path_already_exists::path_already_exists (std::filesystem::path p)
    : std::logic_error {fmt::format ("Temporary path {} already exists.", p)}
    , path {std::move (p)}
  {}
}
