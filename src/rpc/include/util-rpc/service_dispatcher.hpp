// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/serialization/exception.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/asio/spawn.hpp>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace fhg
{
  namespace rpc
  {
    //! A service dispatcher collects multiple \c service_handler and
    //! dispatches requests coming from (multiple) service
    //! providers. The separation allows to provide services via
    //! multiple protocols without redefining handlers.
    //!
    //! Both sides of this are invisible in the \c service_dispatcher
    //! API as the dispatcher is passed to the other classes which
    //! automatically call functions on the dispatcher. Usually it is
    //! used like
    //! ```
    //!   service_dispatcher dispatcher;
    //!   service_handler<proto::fun> handler_fun (dispatcher, fun);
    //!   service_handler<proto::fon> handler_fon (dispatcher, fon);
    //!   service_tcp_provider tcp_provider (io_service, dispatcher);
    //!   service_socket_provider socket_provider (io_service, dispatcher);
    //! ```
    //! which provides the same two services `fun` and `fon` via both,
    //! TCP and UNIX socket.
    //!
    //! \note Handlers have to be uniquely named per
    //! dispatcher. Different dispatchers may have different handlers
    //! with the same name though.
    struct service_dispatcher
    {
    public:
      //! Construct a service dispatcher. The given serialization
      //! functions are used in case a registered handler
      service_dispatcher ( util::serialization::exception::serialization_functions
                         = util::serialization::exception::serialization_functions()
                         );

      //! Not intended to be called manually: Called by service
      //! providers upon incoming function calls.
      void dispatch ( ::boost::asio::yield_context
                    , ::boost::archive::binary_iarchive&
                    , ::boost::archive::binary_oarchive&
                    ) const;

      service_dispatcher (service_dispatcher const&) = delete;
      service_dispatcher (service_dispatcher&&) = delete;
      service_dispatcher& operator= (service_dispatcher const&) = delete;
      service_dispatcher& operator= (service_dispatcher&&) = delete;
      ~service_dispatcher() = default;

    private:
      template<typename> friend struct service_handler;

      std::unordered_map
        < std::string
        , std::function< void ( ::boost::asio::yield_context
                              , ::boost::archive::binary_iarchive&
                              , ::boost::archive::binary_oarchive&
                              )
                       >
        > _handlers;

      util::serialization::exception::serialization_functions _serialization_functions;
    };
  }
}
