// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-rpc/function_description.hpp>
#include <util-rpc/remote_function.hpp>
#include <util-rpc/remote_tcp_endpoint.hpp>
#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_handler.hpp>
#include <util-rpc/service_tcp_provider.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace rpc
  {
    //! \note issue #19: don't start any operations on sockets that
    //! are already closed
    BOOST_AUTO_TEST_CASE (provider_ctor_dtor_in_a_loop)
    {
      service_dispatcher dispatcher;
      util::scoped_boost_asio_io_service_with_threads io_service (1);
      for (int i (0); i < 10000; ++i)
      {
        service_tcp_provider provider (io_service, dispatcher);
      }
    }

    //! \note issue #20: investigate issue #19 for remote_endpoint
    BOOST_AUTO_TEST_CASE (connecting_and_destructing_endpoint_in_a_loop)
    {
      service_dispatcher dispatcher;
      util::scoped_boost_asio_io_service_with_threads io_service (2);
      service_tcp_provider provider (io_service, dispatcher);

      auto const local_endpoint
        (util::connectable_to_address_string (provider.local_endpoint()));

      for (int i (0); i < 5000; ++i)
      {
        remote_tcp_endpoint endpoint (io_service, local_endpoint);
      }
    }
  }
}
