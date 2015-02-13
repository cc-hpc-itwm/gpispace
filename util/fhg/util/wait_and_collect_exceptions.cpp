#include <fhg/util/wait_and_collect_exceptions.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/print_exception.hpp>

#include <sstream>
#include <stdexcept>
#include <string>

namespace fhg
{
  namespace util
  {
    void wait_and_collect_exceptions (std::vector<std::future<void>>& futures)
    {
      std::vector<std::string> accumulated_whats;

      for (std::future<void>& future : futures)
      {
        try
        {
          future.get();
        }
        catch (...)
        {
          std::ostringstream oss;
          fhg::util::print_current_exception (oss, "");
          accumulated_whats.emplace_back (oss.str());
        }
      }

      if (!accumulated_whats.empty())
      {
        throw std::runtime_error (fhg::util::join (accumulated_whats, ", "));
      }
    }
  }
}
