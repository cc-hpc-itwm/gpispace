#include <share/example/stream/test.hpp>

#include <sstream>

BOOST_AUTO_TEST_CASE (share_example_stream_one_module_call)
{
  share_example_stream_test::run
    ( "stream.one_module_call"
    , [] (unsigned long size_slot)
    {
      std::ostringstream topology_description;

      topology_description << "process_and_mark_free:2," << (size_slot + 1);

      return topology_description.str();
    }
    , std::chrono::milliseconds (25)
    , 12.0
    );
}
