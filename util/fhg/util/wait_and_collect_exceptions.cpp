#include <fhg/util/wait_and_collect_exceptions.hpp>

#include <util-generic/print_exception.hpp>

#include <functional>
#include <sstream>
#include <string>

namespace fhg
{
  namespace util
  {
    void throw_collected_exceptions
      (std::vector<std::exception_ptr> const& exceptions)
    {
      if (exceptions.empty())
      {
        return;
      }

      std::ostringstream oss;
      for (std::exception_ptr const& exception : exceptions)
      {
        fhg::util::print_exception (oss, "", exception);
      }
      throw std::runtime_error (oss.str());
    }

    void wait_and_collect_exceptions (std::vector<std::future<void>>& futures)
    {
      apply_for_each_and_collect_exceptions
        (futures, std::bind (&std::future<void>::get, std::placeholders::_1));
    }
  }
}
