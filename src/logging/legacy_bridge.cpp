#include <logging/legacy_bridge.hpp>

namespace fhg
{
  namespace logging
  {
    legacy_bridge::legacy_bridge (unsigned short legacy_port)
      : legacy::receiver (legacy_port)
      , _emitter()
    {}

    endpoint legacy_bridge::local_endpoint() const
    {
      return _emitter.local_endpoint();
    }

    void legacy_bridge::on_legacy (legacy::event const& event)
    {
      _emitter.emit_message (message::from_legacy (event));
    }
  }
}
