// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <boost/format.hpp>
#include <boost/test/unit_test.hpp>

#include <functional>
#include <stdexcept>
#include <typeinfo>

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
        void require_exception ( std::function<void()> f
                               , std::string const& what
                               )
        {
          try
          {
            f();
          }
          catch (E const& e)
          {
            BOOST_REQUIRE_EQUAL (e.what(), what);

            return;
          }
          catch (std::exception const& ex)
          {
            BOOST_FAIL
              ( boost::format ( "missing exception '%1%' with message '%2%'"
                                ", got exception of type '%3%' with message '%4%'"
                              )
              % typeid (E).name()
              % what
              % typeid (ex).name()
              % ex.what()
              );
          }
          catch (...)
          {
            BOOST_FAIL
              ( boost::format ( "missing exception '%1%' with message '%2%'"
                                ", got non std::exception type"
                              )
              % typeid (E).name()
              % what
              );
          }

          BOOST_FAIL
            ( boost::format ( "missing exception '%1%' with message '%2%'"
                              ", got no exception at all"
                            )
            % typeid (E).name()
            % what
            );
        }

        template<typename E>
        void require_exception ( std::function<void()> f
                               , boost::format const& format
                               )
        {
          require_exception<E> (f, format.str());
        }
      }
    }
  }
}
