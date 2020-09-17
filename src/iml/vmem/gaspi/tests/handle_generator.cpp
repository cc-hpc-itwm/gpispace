#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>

#include <iml/vmem/gaspi/pc/type/segment_descriptor.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE ( test_generate )
{
  gpi::pc::memory::handle_generator_t handle_generator (42);

  gpi::pc::type::handle_t globl (handle_generator.next());
  BOOST_CHECK_EQUAL (globl.ident, 42U);
  BOOST_CHECK_EQUAL (globl.cntr, 1U);

  gpi::pc::type::handle_t local (handle_generator.next());
  BOOST_CHECK_EQUAL (local.cntr, 2U);

  gpi::pc::type::handle_id_t id = local;
  BOOST_CHECK_EQUAL (local, id);
}

BOOST_AUTO_TEST_CASE ( test_generate_interleaved )
{
  gpi::pc::memory::handle_generator_t handle_generator (42);

  for (size_t i (0); i < 100; ++i)
  {
    gpi::pc::type::handle_t g (handle_generator.next());
    BOOST_CHECK_EQUAL (g.ident, 42U);
    BOOST_CHECK_EQUAL (g.cntr, 2*(i+1)-1);

    gpi::pc::type::handle_t s (handle_generator.next());
    BOOST_CHECK_EQUAL (s.cntr, 2*(i+1));
  }
}
