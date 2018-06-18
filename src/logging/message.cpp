#include <logging/message.hpp>

#include <stdexcept>

namespace fhg
{
  namespace logging
  {
    message::message (std::string content, std::string category)
      : _content (std::move (content))
      , _category (std::move (category))
    {}

    message message::from_legacy (legacy::event const& event)
    {
      auto const category
        ( event.severity() == legacy::TRACE ? legacy::category_level_trace
        : event.severity() == legacy::INFO ? legacy::category_level_info
        : event.severity() == legacy::WARN ? legacy::category_level_warn
        : event.severity() == legacy::ERROR ? legacy::category_level_error
        : throw std::logic_error ("bad legacy severity")
        );

      return {event.message(), category};
    }
  }
}
