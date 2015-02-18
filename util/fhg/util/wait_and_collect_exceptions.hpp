#pragma once

#include <exception>
#include <future>
#include <vector>

namespace fhg
{
  namespace util
  {
    void throw_collected_exceptions (std::vector<std::exception_ptr> const&);

    template<typename Collection, typename Operation>
      void apply_for_each_and_collect_exceptions
        (Collection&& collection, Operation&& operation)
    {
      std::vector<std::exception_ptr> exceptions;

      for (auto& element : collection)
      {
        try
        {
          operation (element);
        }
        catch (...)
        {
          exceptions.push_back (std::current_exception());
        }
      }

      throw_collected_exceptions (exceptions);
    }

    void wait_and_collect_exceptions (std::vector<std::future<void>>&);
  }
}
