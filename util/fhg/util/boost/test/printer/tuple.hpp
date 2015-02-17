// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <fhg/util/boost/test.hpp>

#include <iostream>
#include <tuple>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      using namespace ::boost;

      namespace test
      {
        namespace printer
        {
          namespace
          {
            template < class tuple_type
                     , size_t index = std::tuple_size<tuple_type>::value - 1
                     >
              struct printer_impl
              {
                static void apply (std::ostream& os, const tuple_type& tuple)
                {
                  printer_impl<tuple_type, index - 1>::apply (os, tuple);
                  os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER
                          (std::get<index> (tuple));
                  os << ", ";
                }
              };

            template <class tuple_type>
              struct printer_impl<tuple_type, 0>
            {
              static void apply (std::ostream& os, const tuple_type& tuple)
              {
                os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (std::get<0> (tuple));
                os << ">";
              }
            };
          }
        }
      }
    }
  }
}

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  (<typename... Args>, std::tuple<Args...>, os, t)
{
  os << "<";
  fhg::util::boost::test::printer::printer_impl<std::tuple<Args...>>::apply
    (os, t);
}
