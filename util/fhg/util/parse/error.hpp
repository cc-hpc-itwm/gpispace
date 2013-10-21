// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_PARSE_ERROR_HPP
#define _FHG_UTIL_PARSE_ERROR_HPP

#include <stdexcept>

#include <fhg/util/backtracing_exception.hpp>
#include <fhg/util/parse/position.hpp>

#include <boost/format.hpp>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      namespace error
      {
#ifndef NO_BACKTRACE_ON_PARSE_ERROR
#define GENERIC_EXCEPTION_BASE_CLASS fhg::util::backtracing_exception
#else
#define GENERIC_EXCEPTION_BASE_CLASS std::runtime_error
#endif

        class generic : public GENERIC_EXCEPTION_BASE_CLASS
        {
        public:
          generic (const std::string& msg, const position& inp)
            : GENERIC_EXCEPTION_BASE_CLASS (inp.error_message (msg))
          {}
          generic (const boost::format& msg, const position& inp)
            : GENERIC_EXCEPTION_BASE_CLASS (inp.error_message (msg.str()))
          {}
        };

#undef GENERIC_EXCEPTION_BASE_CLASS

        class expected : public generic
        {
        public:
          expected (const std::string&, const position&);
          virtual ~expected() throw() {}
        };

        template<typename From, typename To>
          class value_too_big : public generic
        {
        public:
          value_too_big (const From& f, const position& pos)
            : generic ( boost::format ("value %1% larger than maximum %2%")
                      % f
                      % std::numeric_limits<To>::max()
                      , pos
                      )
          {}
          virtual ~value_too_big() throw() {}
        };

        template<typename I>
          class unexpected_digit : public generic
        {
        public:
          unexpected_digit (const position& pos)
            : generic ( boost::format
                        ( "unexpected digit"
                        " (parsed value would be larger than %1%)"
                        )
                      % std::numeric_limits<I>::max()
                      , pos
                      )
          {}
          virtual ~unexpected_digit() throw() {}
        };
      }
    }
  }
}

#endif
