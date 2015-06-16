#pragma once

#include <exception>
#include <string>
#include <vector>

namespace fhg
{
  namespace rif
  {
    struct started_process_promise
    {
      //! \note will eat special arguments and advance
      started_process_promise (int& argc, char**& argv);

      void set_result (std::vector<std::string> messages);
      void set_exception (std::exception_ptr exception);

      static std::string end_sentinel_value() { return "DONE"; }

    private:
      template<typename T> void send (bool result, T const&);

      int _startup_pipe_fd;
    };
  }
}
