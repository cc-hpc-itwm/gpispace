// mirko.rahn@itwm.fraunhofer.de

#include <drts/workflow_response.hpp>

#include <rpc/client.hpp>

#include <we/type/value/serialize.hpp>

#include <boost/thread/scoped_thread.hpp>

namespace gspc
{
  void workflow_response ( std::string const& trigger_address
                         , unsigned short const& trigger_port
                         , pnet::type::value::value_type const& value
                         )
  {
    struct io_service_with_work_thread_and_stop_on_scope_exit
    {
      io_service_with_work_thread_and_stop_on_scope_exit()
        : service()
        , _work (service)
        , _thread ([this] { service.run(); })
      {}
      ~io_service_with_work_thread_and_stop_on_scope_exit()
      {
        service.stop();
      }
      boost::asio::io_service service;

    private:
      boost::asio::io_service::work const _work;
      boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable> const
        _thread;
    } io_service_client;

    fhg::rpc::remote_endpoint endpoint
      ( io_service_client.service
      , trigger_address
      , trigger_port
      , fhg::util::serialization::exception::serialization_functions()
      );

    fhg::rpc::sync_remote_function<void (pnet::type::value::value_type)>
      (endpoint, "set_result") (value);
  }
}
