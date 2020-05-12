#pragma once

#include <util-generic/testing/random.hpp>

#include <logging/message.hpp>

#include <ostream>
#include <tuple>

namespace fhg
{
  namespace logging
  {
    //! \note operator== and operator<< are not in an anonmyous
    //! namespace for lookup reasons: they are used from a boost
    //! namespace, so only an anonymous namespace in that boost
    //! namespace would be used.
    bool operator== (message const& lhs, message const& rhs)
    {
      return std::tie (lhs._content, lhs._category)
        == std::tie (rhs._content, rhs._category);
    }
    std::ostream& operator<< (std::ostream& os, message const& x)
    {
      return os << "content=" << x._content << ", "
                << "category=" << x._category;
    }
  }

  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        template<>
          struct random_impl<logging::message, void>
        {
          logging::message operator()() const
          {
            random<std::string> random_string;
            return {random_string(), random_string()};
          }
        };
      }
    }
  }
}
