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
      //! \note will remove special arguments
      started_process_promise (int& argc, char**& argv);
      ~started_process_promise();

      void set_result (std::vector<std::string> messages);
      void set_exception (std::exception_ptr exception);

      static std::string end_sentinel_value() { return "DONE"; }

    private:
      template<typename T> void send (bool result, T const&);

      char** _original_argv;
      int& _argc;
      char**& _argv;
      std::vector<char*> _replacement_argv;
      int _startup_pipe_fd;
    };
  }
}
