#define BOOST_TEST_MODULE GpiSpaceHandleGeneratorTest
#include <boost/test/unit_test.hpp>

#include <fhglog/LogMacros.hpp>

#include <gpi-space/pc/memory/handle_generator.hpp>
#include <gpi-space/pc/type/segment_descriptor.hpp>

BOOST_AUTO_TEST_CASE ( test_generate )
{
  gpi::pc::memory::handle_generator_t handle_generator (42);

  gpi::pc::type::handle_t globl
      (handle_generator.next (gpi::pc::type::segment::SEG_GPI));
  BOOST_CHECK_EQUAL (globl.type, gpi::pc::type::segment::SEG_GPI);
  BOOST_CHECK_EQUAL (globl.gpi.ident, 42U);
  BOOST_CHECK_EQUAL (globl.gpi.cntr, 1U);
  std::cerr << "gpi=" << globl << std::endl;

  gpi::pc::type::handle_t local
      (handle_generator.next (gpi::pc::type::segment::SEG_SHM));
  BOOST_CHECK_EQUAL (local.type, gpi::pc::type::segment::SEG_SHM);
  BOOST_CHECK_EQUAL (local.shm.cntr, 1U);
  std::cerr << "shm=" << local << std::endl;

  gpi::pc::type::handle_id_t id = local;
  BOOST_CHECK_EQUAL (local, id);
}

BOOST_AUTO_TEST_CASE ( test_generate_interleaved )
{
  gpi::pc::memory::handle_generator_t handle_generator (42);

  for (size_t i (0); i < 100; ++i)
  {
    gpi::pc::type::handle_t g
        (handle_generator.next (gpi::pc::type::segment::SEG_GPI));
    BOOST_CHECK_EQUAL (g.type, gpi::pc::type::segment::SEG_GPI);
    BOOST_CHECK_EQUAL (g.gpi.ident, 42U);
    BOOST_CHECK_EQUAL (g.gpi.cntr, i+1);

    gpi::pc::type::handle_t s
        (handle_generator.next (gpi::pc::type::segment::SEG_SHM));
    BOOST_CHECK_EQUAL (s.type, gpi::pc::type::segment::SEG_SHM);
    BOOST_CHECK_EQUAL (s.shm.cntr, i+1);
  }
}