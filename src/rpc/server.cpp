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
      network::buffer_type message
        (header->buffer, header->buffer + header->buffer_size);

      util::parse::position_vector_of_char pos (message);
      const std::size_t len (util::read_size_t (pos));
      util::parse::require::require (pos, ' ');

      std::string const function_name (pos.eat (len));

      decltype (_handlers)::const_iterator const handler
        (_handlers.find (function_name));

      if (handler == _handlers.end())
      {
        throw std::logic_error ("function '" + function_name + "' does not exist");
      }

      network::buffer_type response (handler->second (pos));

      packet_header response_header (header->message_id, response.size());
      network::buffer_type response_packet (response);
      response_packet.insert ( response_packet.begin()
                             , (char*)&response_header
                             , (char*)&response_header + sizeof (response_header)
                             );

      connection->send (response_packet);
    }

    service_handler::service_handler
        ( service_dispatcher& manager
        , std::string name
        , std::function<network::buffer_type (util::parse::position&)> handler
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
