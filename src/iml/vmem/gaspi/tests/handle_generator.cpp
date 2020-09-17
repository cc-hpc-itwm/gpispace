#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE ( test_generate )
{
  gpi::pc::memory::handle_generator_t handle_generator (42);

  BOOST_CHECK_EQUAL
    (handle_generator.next(), gpi::pc::type::handle_t (42U, 1U));

  gpi::pc::type::handle_t local (handle_generator.next());
  BOOST_CHECK_EQUAL (local, gpi::pc::type::handle_t (42U, 2U));

  gpi::pc::type::handle_id_t id = local;
  BOOST_CHECK_EQUAL (local, id);
}

BOOST_AUTO_TEST_CASE ( test_generate_interleaved )
{
  gpi::pc::memory::handle_generator_t handle_generator (42);

  for (size_t i (0); i < 100; ++i)
  {
    BOOST_CHECK_EQUAL
      (handle_generator.next(), gpi::pc::type::handle_t (42U, 2*(i+1)-1));
    BOOST_CHECK_EQUAL
      (handle_generator.next(), gpi::pc::type::handle_t (42U, 2*(i+1)));
  }
}
