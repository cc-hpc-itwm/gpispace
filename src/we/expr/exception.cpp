// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/exception.hpp>

#include <boost/format.hpp>

namespace expr
{
  namespace exception
  {
    namespace parse
    {
      exception::exception (const std::string& msg, const std::size_t k)
        : std::runtime_error
            ((boost::format ("parse error [%1%]: %2%") % k % msg).str())
        , eaten (k)
      {}

      expected::expected (const std::string& what, const std::size_t k)
        : exception ("expected one of: " + what, k)
      {}

      misplaced::misplaced (const std::string& what, const std::size_t k)
        : exception ( "misplaced " + what + ", operator expected"
                    , k - what.length()
                    )
      {}

      unterminated::unterminated ( const std::string& what
                                 , const std::size_t open
                                 , const std::size_t k
                                 )
        : exception
          ( ( boost::format ("unterminated %1%, opened at: %2%")
            % what
            % open
            ).str()
          , k
          )
      {}

      missing::missing (const std::string& what, const std::size_t k)
        : exception ("missing " + what, k)
      {}
    }

    namespace eval
    {
      divide_by_zero::divide_by_zero()
        : std::runtime_error ("divide by zero")
      {}

      type_error::type_error (const std::string& msg)
        : std::runtime_error ("type error: " + msg)
      {}
      type_error::type_error (const boost::format& msg)
        : std::runtime_error ("type error: " + msg.str())
      {}

      negative_exponent::negative_exponent()
        : std::runtime_error ("negative exponent")
      {}
    }
  }
}
