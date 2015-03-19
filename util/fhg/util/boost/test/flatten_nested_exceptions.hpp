// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <util-generic/print_exception.hpp>

#include <boost/test/unit_test_monitor.hpp>
#include <boost/test/unit_test_suite.hpp>

#include <sstream>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      using namespace ::boost;
      namespace unit_test
      {
        using namespace ::boost::unit_test;

        //! \note HACK: relies on being called during exception
        //! handling due to needing the unchanged type for
        //! rethrowing. if called from a exception_ptr or alike, it
        //! will fail due to std::current_exception() being something
        //! else or null.
        struct flatten_nested_exceptions
        {
          flatten_nested_exceptions()
          {
            unit_test_monitor.register_exception_translator<std::exception>
              ( [] (std::exception const& ex)
                {
                  if (!dynamic_cast<std::nested_exception const*> (&ex))
                  {
                    throw;
                  }

                  std::ostringstream ss;
                  print_current_exception (ss, "");
                  struct flattened_exception : std::exception
                  {
                    std::string _what;
                    flattened_exception (std::string const& what)
                      : _what (what)
                    {}
                    virtual const char* what() const noexcept override
                    {
                      return _what.c_str();
                    }
                  } flattened (ss.str());
                  throw flattened;
                }
              );
          }
        };

        BOOST_GLOBAL_FIXTURE (flatten_nested_exceptions)
      }
    }
  }
}
