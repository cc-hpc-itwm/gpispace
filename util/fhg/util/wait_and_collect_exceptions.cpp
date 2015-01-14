#include <fhg/util/wait_and_collect_exceptions.hpp>

#include <fhg/util/join.hpp>

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
        catch (std::exception const& ex)
        {
          accumulated_whats.emplace_back (ex.what());
        }
      }

      if (!accumulated_whats.empty())
      {
        throw std::runtime_error (fhg::util::join (accumulated_whats, ", "));
      }
    }
  }
}
