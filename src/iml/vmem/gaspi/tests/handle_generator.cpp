#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>

#include <iml/vmem/gaspi/pc/type/segment_descriptor.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE ( test_generate )
{
  gpi::pc::memory::handle_generator_t handle_generator (42);

  gpi::pc::type::handle_t globl
      (handle_generator.next (gpi::pc::type::segment::SEG_GASPI));
  BOOST_CHECK_EQUAL (globl.gpi.ident, 42U);
  BOOST_CHECK_EQUAL (globl.gpi.cntr, 1U);

  gpi::pc::type::handle_t local
      (handle_generator.next (gpi::pc::type::segment::SEG_SHM));
  BOOST_CHECK_EQUAL (local.shm.cntr, 1U);

  gpi::pc::type::handle_id_t id = local;
  BOOST_CHECK_EQUAL (local, id);
}

BOOST_AUTO_TEST_CASE ( test_generate_interleaved )
{
  gpi::pc::memory::handle_generator_t handle_generator (42);

  for (size_t i (0); i < 100; ++i)
  {
    gpi::pc::type::handle_t g
        (handle_generator.next (gpi::pc::type::segment::SEG_GASPI));
    BOOST_CHECK_EQUAL (g.gpi.ident, 42U);
    BOOST_CHECK_EQUAL (g.gpi.cntr, i+1);

    gpi::pc::type::handle_t s
        (handle_generator.next (gpi::pc::type::segment::SEG_SHM));
    BOOST_CHECK_EQUAL (s.shm.cntr, i+1);
  }
}
