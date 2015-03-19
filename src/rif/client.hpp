// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <rif/entry_point.hpp>
#include <rif/protocol.hpp>

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
                    , fhg::util::serialization::exception::serialization_functions()
                    )
        , execute_and_get_startup_messages (_endpoint)
        , kill (_endpoint)
        , start_vmem (_endpoint)
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
      rpc::remote_function<protocol::execute_and_get_startup_messages>
        execute_and_get_startup_messages;
      rpc::remote_function<protocol::kill> kill;
      rpc::remote_function<protocol::start_vmem> start_vmem;
    };
  }
}
