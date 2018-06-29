#pragma once

#include <logging/legacy/receiver.hpp>
#include <logging/stream_emitter.hpp>
#include <logging/tcp_receiver.hpp>

namespace fhg
{
  namespace logging
  {
    class legacy_bridge final : public legacy::receiver
    {
    public:
      legacy_bridge (unsigned short legacy_port);

      tcp_endpoint local_endpoint() const;

      using receiver = tcp_receiver;

    private:
      virtual void on_legacy (legacy::event const&) override;

      stream_emitter _emitter;
    };
  }
}
