// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_BOOST_TEST_REQUIRE_EXCEPTION_HPP
#define FHG_UTIL_BOOST_TEST_REQUIRE_EXCEPTION_HPP

#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/test/unit_test.hpp>

#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      using namespace ::boost;

      namespace test
      {
        template<typename E>
        void require_exception ( boost::function<void()> f
                               , std::string const& name
                               , std::string const& what
                               )
        {
          try
          {
            f();

            BOOST_FAIL
              ( boost::format ( "missing exception '%1%' with message '%2%'"
                                ", got no exception at all"
                              )
              % name
              % what
              );
          }
          catch (E const& e)
          {
            BOOST_REQUIRE_EQUAL (e.what(), what);
          }
          catch (...)
          {
            BOOST_FAIL
              ( boost::format ( "missing exception '%1%' with message '%2%'"
                                ", got exception of different type"
                              )
              % name
              % what
              );
          }
        }

      }
    }
  }
}

#endif
