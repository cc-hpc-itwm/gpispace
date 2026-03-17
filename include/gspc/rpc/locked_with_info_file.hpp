#pragma once

#include <gspc/rpc/remote_tcp_endpoint.hpp>
#include <gspc/rpc/service_tcp_provider.hpp>
#include <gspc/detail/export.hpp>

#include <gspc/util/filesystem_lock_directory.hpp>
#include <gspc/util/scoped_file_with_content.hpp>

#include <filesystem>
#include <functional>



    //! This namespace contains wrappers around \c
    //! service_tcp_provider and \c remote_tcp_endpoint which are
    //! exchanging connection information via the filesystem. The \c
    //! locked_with_info_file::server expects a \c
    //! gspc::util::filesystem_lock_directory to already been created, which
    //! ensures that there is only exactly one server listening at the
    //! given path. Many \c locked_with_info_file::client can then be
    //! created by passing the path used on server construction.
    namespace gspc::rpc::locked_with_info_file
    {
      namespace error
      {
        struct GSPC_EXPORT failed_to_create_server : std::runtime_error
        {
          std::filesystem::path directory;
          failed_to_create_server (decltype (directory) dir);
        };
        struct GSPC_EXPORT failed_to_create_client : std::runtime_error
        {
          std::filesystem::path directory;
          failed_to_create_client (decltype (directory) dir);
        };
        struct GSPC_EXPORT bad_server_description : std::logic_error
        {
          bad_server_description();
        };
      }

      struct GSPC_EXPORT server : public service_tcp_provider
      {
        server ( ::boost::asio::io_service&
               , service_dispatcher&
               , gspc::util::filesystem_lock_directory const&
               );

      private:
        gspc::util::scoped_file_with_content const _server;
      };

      struct GSPC_EXPORT client : public remote_tcp_endpoint
      {
        client ( ::boost::asio::io_service&
               , std::filesystem::path const&
               , gspc::util::serialization::exception::serialization_functions
               = gspc::util::serialization::exception::serialization_functions()
               );
      };
    }
