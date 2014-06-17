// bernd.loerwald@itwm.fraunhofer.de

#include <playground/bl/rpc/server.hpp>

void service_dispatcher::dispatch
  (connection_type* connection, buffer_type packet) const
{
  packet_header* header ((packet_header*)packet.data());
  buffer_type message (header->buffer, header->buffer + header->buffer_size);

  fhg::util::parse::position_vector_of_char pos (message);
  const std::size_t len (fhg::util::read_size_t (pos));
  fhg::util::parse::require::require (pos, ' ');

  buffer_type response (_handlers.at (pos.eat (len)) (pos));

  packet_header response_header (header->message_id, response.size());
  buffer_type response_packet (response);
  response_packet.insert ( response_packet.begin()
                         , (char*)&response_header
                         , (char*)&response_header + sizeof (response_header)
                         );

  connection->send (response_packet);
}

service_handler::service_handler ( service_dispatcher& manager
                                 , std::string name
                                 , std::function<buffer_type (fhg::util::parse::position&)> handler
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
