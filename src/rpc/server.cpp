// bernd.loerwald@itwm.fraunhofer.de

#include <rpc/server.hpp>

namespace fhg
{
  namespace rpc
  {
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

      std::string return_value (handler->second (call_function->arguments()));
      network::buffer_type response (return_value.begin(), return_value.end());

      packet_header const response_header (header->message_id, response.size());
      connection->send ({{ (char*)&response_header
                         , (char*)&response_header + sizeof (response_header)
                         }
                        , std::move (response)
                        }
                       );
    }

    service_handler::service_handler
        ( service_dispatcher& manager
        , std::string name
        , std::function<std::string (std::string)> handler
        )
      : _manager (manager)
      , _name (name)
    {
      if (_manager._handlers.find (_name) != _manager._handlers.end())
      {
        throw std::runtime_error ("already have service with name " + _name);
      }
      _manager._handlers.insert (std::make_pair (_name, handler));
    }
    service_handler::~service_handler()
    {
      _manager._handlers.erase (_name);
    }
  }
}
