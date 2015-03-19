// mirko.rahn@itwm.fhg.de

#pragma once

#include <fhg/util/boost/test.hpp>
#include <util-generic/print_container.hpp>

#include <functional>
#include <string>

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
          template<typename T>
          class print_element_default
          {
          public:
            void operator() (std::ostream& os, T const& x) const
            {
              os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (x);
            }
          };

          template<typename Container>
          std::ostream& container
            ( std::ostream& os
            , Container const& c
            , std::string const& prefix
            , std::string const& suffix
            , std::function <void ( std::ostream&
                                  , typename Container::value_type const&
                                  )
                            > const& print_element
              = print_element_default<typename Container::value_type>()
            )
          {
            return fhg::util::print_container<Container>
              ( os, "", prefix, ",", suffix, c
              , std::bind (print_element, std::ref (os), std::placeholders::_1)
              );
          }
        }
      }
    }
  }
}
