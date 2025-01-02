// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <testing/make.hpp>
#include <testing/parse_command_line.hpp>
#include <testing/scoped_nodefile_from_environment.hpp>
#include <testing/shared_directory.hpp>
#include <testing/source_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/vector.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/random/integral.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <algorithm>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

BOOST_AUTO_TEST_CASE (number_of_tokens_condition_and_number_on_the_same_place)
{
  ::boost::program_options::options_description options_description;

  options_description.add (test::options::shared_directory());
  options_description.add (test::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());

  ::boost::program_options::variables_map vm
    ( test::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / "share_example_number_of_tokens_condition_and_number_on_the_same_place"
    );

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net const make
    ( installation
    , "condition_and_number_on_the_same_place"
    , test::source_directory (vm)
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  gspc::scoped_runtime_system const drts { vm
                                         , installation
                                         , ""
                                         , rifds.entry_points()
                                         };

  auto const n {fhg::util::testing::random<unsigned long>{} (1000UL, 0UL)};

  std::multimap<std::string, pnet::type::value::value_type> const result
    { gspc::client (drts).put_and_run ( gspc::workflow (make.pnet())
                                        , { {"n", n}
                                          }
                                      )
    };

  BOOST_REQUIRE_EQUAL (result.size(), n + 2UL);

  auto const result_values
    { [&] (auto key)
      {
        auto const [begin, end] {result.equal_range (key)};

        auto vs {std::vector<unsigned long>{}};

        std::transform
          ( begin, end
          , std::back_inserter (vs)
          , [] (auto key_value)
            {
              return ::boost::get<unsigned long> (key_value.second);
            }
          );

        return vs;
      }
    };

  {
    auto const sum {result_values ("sum")};

    BOOST_REQUIRE_EQUAL (sum.size(), 1);
    BOOST_REQUIRE_EQUAL (sum.front(), n * (n - 1UL));
  }

  {
    auto const count {result_values ("count")};

    BOOST_REQUIRE_EQUAL (count.size(), 1);
    BOOST_REQUIRE_GE (count.front(), n * (n + 1UL));
  }

  {
    auto const is {result_values ("i")};

    BOOST_REQUIRE_EQUAL (is.size(), n);

    auto const uneven
      { std::invoke
        ( [&]
          {
            struct Uneven
            {
              [[nodiscard]] auto operator()() noexcept -> unsigned long
              {
                FHG_UTIL_FINALLY ([&] { _i += 2UL; });

                return _i;
              }

            private:
              unsigned long _i {1UL};
            };

            auto _uneven {std::vector<unsigned long>{}};
            _uneven.reserve (n);

            std::generate_n (std::back_inserter (_uneven), n, Uneven{});

            return _uneven;
          }
        )
      };

    FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (is, uneven);
  }
}
