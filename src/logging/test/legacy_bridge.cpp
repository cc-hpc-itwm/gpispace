#include <logging/legacy/emitter.hpp>
#include <logging/legacy_bridge.hpp>
#include <logging/message.hpp>
#include <logging/test/message.hpp>

#include <test/hopefully_free_port.hpp>

#include <util-generic/hostname.hpp>
#include <util-generic/testing/printer/future.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <thread>

namespace fhg
{
  namespace logging
  {
    namespace
    {
      std::array<char const* const, 4> const legacy_level_categories
        { legacy::category_level_trace
        , legacy::category_level_info
        , legacy::category_level_warn
        , legacy::category_level_error
        };

      std::array<void (legacy::emitter::*) (std::string const&), 4> const
        legacy_emitter_emit_functions { &legacy::emitter::trace
                                      , &legacy::emitter::info
                                      , &legacy::emitter::warn
                                      , &legacy::emitter::error
                                      };
    }

    BOOST_DATA_TEST_CASE
      ( legacy_bridge_forwards
      , boost::unit_test::data::make (legacy_level_categories)
      ^ boost::unit_test::data::make (legacy_emitter_emit_functions)
      , expected_category
      , emit_function
      )
    {
      auto const sent_content (util::testing::random<std::string>{}());
      message const expected (sent_content, expected_category);

      std::promise<message> received;
      auto received_future (received.get_future());

      test::hopefully_free_port legacy_port;
      legacy_bridge const bridge (legacy_port.release());
      legacy_bridge::receiver const receiver
        ( bridge.local_endpoint()
        , [&] (message const& m)
          {
            received.set_value (m);
          }
        );

      legacy::emitter emitter (util::hostname(), legacy_port);
      (emitter.*emit_function) (sent_content);

      BOOST_REQUIRE_EQUAL
        ( received_future.wait_for (std::chrono::milliseconds (200))
        , std::future_status::ready
        );

      BOOST_REQUIRE_EQUAL (received_future.get(), expected);
    }
  }
}
