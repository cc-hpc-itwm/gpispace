#pragma once

#include <stdexcept>

#include <iml/util/parse/position.hpp>

#include <boost/format.hpp>

namespace fhg
{
  namespace iml
{
  namespace util
  {
    namespace parse
    {
      namespace error
      {
        class generic : public std::runtime_error
        {
        public:
          generic (const std::string& msg, const position& inp)
            : std::runtime_error (inp.error_message (msg))
          {}
          generic (const boost::format& msg, const position& inp)
            : std::runtime_error (inp.error_message (msg.str()))
          {}
        };

        class expected : public generic
        {
        public:
          expected (const std::string&, const position&);
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
        };
      }
    }
  }
}
}
