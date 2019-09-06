#pragma once

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

namespace gspc
{
  class installation;
  class scoped_runtime_system;
  using Certificates = boost::optional<boost::filesystem::path>;
}
