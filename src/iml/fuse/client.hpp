#pragma once

#include <boost/filesystem/path.hpp>

namespace iml
{
  namespace fuse
  {
    //! Mounts and blocks.
    void client ( boost::filesystem::path const& mountpoint
                , boost::filesystem::path const& iml_socket
                );
  }
}
