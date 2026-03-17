#include <gspc/util/exit_status.hpp>
#include <gspc/util/system.hpp>

#include <fmt/core.h>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <sys/wait.h>


  namespace gspc::util
  {
    template<>
      void system<Command> (Command command)
    {
      if (int ec = std::system (command.c_str()))
      {
        throw std::runtime_error
          { fmt::format ( "Could not execute '{}': {}"
                        , command
                        , std::strerror (wexitstatus (ec))
                        )
          };
      }

      return;
    }
  }
