#define BOOST_TEST_MODULE job_id

#include <boost/test/unit_test.hpp>
#include <sdpa/types.hpp>
#include <sdpa/id_generator.hpp>

BOOST_AUTO_TEST_CASE (id_generator)
{
  sdpa::id_generator generator ("job");

  const sdpa::job_id_t jid1 (generator.next());
  const sdpa::job_id_t jid2 (generator.next());
  const sdpa::job_id_t jid3 (generator.next());

  BOOST_REQUIRE_NE (jid1, jid2);
  BOOST_REQUIRE_NE (jid1, jid3);
  BOOST_REQUIRE_NE (jid2, jid3);
  BOOST_REQUIRE_EQUAL (jid1, jid1);
  BOOST_REQUIRE_EQUAL (jid2, jid2);
  BOOST_REQUIRE_EQUAL (jid3, jid3);
}
