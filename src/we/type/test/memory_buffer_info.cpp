#include <we/type/memory_buffer_info_t.hpp>

#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (size_and_alignment_as_expressions)
{
  expr::eval::context context;
  std::unordered_map<std::string, unsigned long> expected;

  unsigned long const size
    {fhg::util::testing::random<unsigned long>()()};
  auto const size_expr (fhg::util::testing::random_identifier());
  context.bind_and_discard_ref ({size_expr}, size);

  unsigned long const alignment
     {fhg::util::testing::random<unsigned long>()()};
  auto const alignment_expr (fhg::util::testing::random_identifier());
    context.bind_and_discard_ref ({alignment_expr}, alignment);

  we::type::memory_buffer_info_t memory_buffer_info
    ("${" + size_expr + "}", "${" + alignment_expr + "}");

  BOOST_REQUIRE_EQUAL (memory_buffer_info.size (context), size);
  BOOST_REQUIRE_EQUAL (memory_buffer_info.alignment (context), alignment);
}

BOOST_AUTO_TEST_CASE (size_and_alignment_as_constants)
{
  expr::eval::context context;
  std::unordered_map<std::string, unsigned long> expected;

  unsigned long const size
    {fhg::util::testing::random<unsigned long>()()};

  unsigned long const alignment
     {fhg::util::testing::random<unsigned long>()()};

  we::type::memory_buffer_info_t memory_buffer_info
    ( std::to_string (size) + "UL"
    , std::to_string (alignment) + "UL"
    );

  BOOST_REQUIRE_EQUAL (memory_buffer_info.size (context), size);
  BOOST_REQUIRE_EQUAL (memory_buffer_info.alignment (context), alignment);
}
