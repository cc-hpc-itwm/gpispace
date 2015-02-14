// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <rif/entry_point.hpp>

#include <rpc/client.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <unistd.h>

namespace fhg
{
  namespace rif
  {
    class client
    {
    public:
      client (fhg::rif::entry_point const& entry_point)
        : _io_service()
        , _io_service_work (_io_service)
        , _io_service_thread ([this] { _io_service.run(); })
        , _endpoint ( _io_service
                    , entry_point.hostname, entry_point.port
                    , fhg::rpc::exception::serialization_functions()
                    )
        , execute_and_get_startup_messages
            (_endpoint, "execute_and_get_startup_messages")
        , kill (_endpoint, "kill")
      {}

    private:
      boost::asio::io_service _io_service;
      boost::asio::io_service::work const _io_service_work;
      boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable> const
        _io_service_thread;

      fhg::rpc::remote_endpoint _endpoint;

      struct stop_io_service_on_scope_exit
      {
        ~stop_io_service_on_scope_exit()
        {
          _io_service.stop();
        }
        boost::asio::io_service& _io_service;
      } _stop_io_service_on_scope_exit {_io_service};

    public:
      rpc::remote_function
        < std::pair<pid_t, std::vector<std::string>>
            ( boost::filesystem::path command
            , std::vector<std::string> arguments
            , std::unordered_map<std::string, std::string> environment
            )
        > execute_and_get_startup_messages;
      rpc::remote_function<void (std::vector<pid_t> pids)> kill;
    };
  }
}
