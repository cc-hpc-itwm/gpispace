// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <rpc/remote_tcp_endpoint.hpp>
#include <rpc/service_tcp_provider.hpp>

#include <util-generic/filesystem_lock_directory.hpp>
#include <util-generic/scoped_file_with_content.hpp>

#include <boost/filesystem/path.hpp>

#include <functional>

namespace fhg
{
  namespace rpc
  {
    //! This namespace contains wrappers around \c
    //! service_tcp_provider and \c remote_tcp_endpoint which are
    //! exchanging connection information via the filesystem. The \c
    //! locked_with_info_file::server expects a \c
    //! util::filesystem_lock_directory to already been created, which
    //! ensures that there is only exactly one server listening at the
    //! given path. Many \c locked_with_info_file::client can then be
    //! created by passing the path used on server construction.
    namespace locked_with_info_file
    {
      namespace error
      {
        struct failed_to_create_server : std::runtime_error
        {
          boost::filesystem::path directory;
          failed_to_create_server (decltype (directory) dir);
        };
        struct failed_to_create_client : std::runtime_error
        {
          boost::filesystem::path directory;
          failed_to_create_client (decltype (directory) dir);
        };
        struct bad_server_description : std::logic_error
        {
          bad_server_description();
        };
      }

      struct server : public service_tcp_provider
      {
        server ( boost::asio::io_service&
               , service_dispatcher&
               , util::filesystem_lock_directory const&
               );

      private:
        util::scoped_file_with_content const _server;
      };

      struct client : public remote_tcp_endpoint
      {
        client ( boost::asio::io_service&
               , boost::filesystem::path const&
               , util::serialization::exception::serialization_functions
               = util::serialization::exception::serialization_functions()
               );
      };
    }
  }
}
