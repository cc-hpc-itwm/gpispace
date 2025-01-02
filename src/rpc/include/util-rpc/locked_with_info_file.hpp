// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-rpc/remote_tcp_endpoint.hpp>
#include <util-rpc/service_tcp_provider.hpp>

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
          ::boost::filesystem::path directory;
          failed_to_create_server (decltype (directory) dir);
        };
        struct failed_to_create_client : std::runtime_error
        {
          ::boost::filesystem::path directory;
          failed_to_create_client (decltype (directory) dir);
        };
        struct bad_server_description : std::logic_error
        {
          bad_server_description();
        };
      }

      struct server : public service_tcp_provider
      {
        server ( ::boost::asio::io_service&
               , service_dispatcher&
               , util::filesystem_lock_directory const&
               );

      private:
        util::scoped_file_with_content const _server;
      };

      struct client : public remote_tcp_endpoint
      {
        client ( ::boost::asio::io_service&
               , ::boost::filesystem::path const&
               , util::serialization::exception::serialization_functions
               = util::serialization::exception::serialization_functions()
               );
      };
    }
  }
}
