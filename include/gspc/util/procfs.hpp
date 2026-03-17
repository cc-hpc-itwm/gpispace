#pragma once

#include <filesystem>

#include <list>
#include <string>



    //! \note races everywhere
    namespace gspc::util::procfs
    {
      struct entry
      {
        entry (pid_t);
        entry (std::string const& pid)
          : entry (std::stoi (pid))
        {}

        pid_t pid() const
        {
          return _pid;
        }
        std::list<std::string> const& command_line() const
        {
          return _command_line;
        }

      private:
        pid_t const _pid;
        std::list<std::string> const _command_line;
      };

      std::list<procfs::entry> entries();
    }
