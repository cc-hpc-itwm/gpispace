#pragma once

#include <future>
#include <stdexcept>



    namespace gspc::util::cxx17
    {
      inline std::future_error make_future_error (std::error_code const& ec)
      {
        if (ec.category() != std::future_category())
        {
          throw std::invalid_argument
            ( "called make_future_error with an error_code that is not of "
              "the future category"
            );
        }
        return std::future_error {static_cast<std::future_errc> (ec.value())};
      }
    }
