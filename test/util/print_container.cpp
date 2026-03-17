#include <boost/test/unit_test.hpp>

#include <gspc/util/join.hpp>
#include <gspc/util/print_container.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/test_case.hpp>

#include <cstddef>
#include <iostream>
#include <list>
#include <string>
#include <vector>

GSPC_TESTING_TEMPLATED_CASE_T ( print_container
                                  , Container
                                  , std::list<int>
                                  , std::list<std::string>
                                  , std::vector<int>
                                  , std::vector<std::string>
                                  )
{
  for (std::size_t n (0); n < 10; ++n) BOOST_TEST_CONTEXT ("n = " << n)
  {
    Container const container {gspc::testing::randoms<Container> (n)};
    std::string const open {gspc::testing::random<std::string>()()};
    std::string const separator {gspc::testing::random<std::string>()()};
    std::string const close {gspc::testing::random<std::string>()()};

    using join = gspc::util::join_reference<Container, std::string>;

    BOOST_REQUIRE_EQUAL
      ( open + join (container, separator).string() + close
      , gspc::util::print_container (open, separator, close, container).string()
      );
  }
}


  namespace gspc::util
  {
    namespace
    {
      struct NeitherCopyNorMoveable
      {
        // https://bugs.llvm.org/show_bug.cgi?id=25084 prevents from writing
        // NeitherCopyNorMoveable() = default;
        NeitherCopyNorMoveable() {}
        NeitherCopyNorMoveable (NeitherCopyNorMoveable const&) = delete;
        NeitherCopyNorMoveable (NeitherCopyNorMoveable&&) = delete;
        NeitherCopyNorMoveable& operator= (NeitherCopyNorMoveable const&) = delete;
        NeitherCopyNorMoveable& operator= (NeitherCopyNorMoveable&&) = delete;
        // https://bugs.llvm.org/show_bug.cgi?id=25084 prevents from writing
        // ~NeitherCopyNorMoveable() = default;
        ~NeitherCopyNorMoveable() {}
      };
      std::ostream& operator<< (std::ostream& os, NeitherCopyNorMoveable const&)
      {
        return os << '.';
      }
    }

    BOOST_AUTO_TEST_CASE (noncopyormoveables_can_be_printed)
    {
      std::vector<NeitherCopyNorMoveable> xs
        (gspc::testing::random<std::size_t>{} (1000));

      std::string const expected (xs.size(), '.');
      auto const output (print_container ({}, {}, {}, xs).string());

      BOOST_REQUIRE_EQUAL (output, expected);
    }
  }
