// bernd.loerwald@itwm.fraunhofer.de

#include <rpc/server.hpp>

namespace fhg
{
  namespace rpc
  {
    service_dispatcher::service_dispatcher
        (exception::serialization_functions functions)
      : _serialization_functions (std::move (functions))
    {}

    void service_dispatcher::dispatch
      (network::connection_type* connection, network::buffer_type packet) const
    {
      packet_header* header ((packet_header*)packet.data());
      protocol::call_function* call_function
        ((protocol::call_function*)header->buffer);

      decltype (_handlers)::const_iterator const handler
        (_handlers.find (call_function->function_name()));

      if (handler == _handlers.end())
      {
        throw std::logic_error
          ("function '" + call_function->function_name() + "' does not exist");
      }

      std::pair<bool, std::string> result
        ( [&]()
        {
          try
          {
            return std::make_pair
              (false, handler->second (call_function->arguments()));
          }
          catch (...)
          {
            return std::make_pair
              ( true
              , exception::serialize ( std::current_exception()
                                     , _serialization_functions
                                     , _aggregated_serialization_functions
                                     )
              );
          }
        }()
        );

      network::buffer_type response
        (protocol::function_call_result::required_size (result.second));
      new (response.data()) protocol::function_call_result
        (std::move (result.second), result.first);

      packet_header const response_header (header->message_id, response.size());
      connection->send ({{ (char*)&response_header
                         , (char*)&response_header + sizeof (response_header)
                         }
                        , std::move (response)
                        }
                       );
    }
  }
}