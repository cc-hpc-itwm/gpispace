#include <test/hopefully_free_port.hpp>

#include <util-generic/cxx14/make_unique.hpp>

namespace test
{
  hopefully_free_port::hopefully_free_port()
    : _io_service()
    , _dispatcher()
    , _provider ( fhg::util::cxx14::make_unique
                    <fhg::rpc::service_tcp_provider_with_deferred_start>
                      (_io_service, _dispatcher)
                )
    , _port (_provider->local_endpoint().port())
  {}

  hopefully_free_port::operator unsigned short() const
  {
    return _port;
  }
  unsigned short hopefully_free_port::release()
  {
    _provider.reset();
    return *this;
  }
}
