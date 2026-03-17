#include <gspc/util/procfs.hpp>
#include <gspc/util/read_file.hpp>
#include <gspc/util/split.hpp>

#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <utility>



    namespace gspc::util::procfs
    {
      namespace
      {
        std::filesystem::path proc()
        {
          return "/proc";
        }
      }

      entry::entry (pid_t pid)
        : _pid (pid)
        , _command_line
          ( gspc::util::split<std::string, std::string>
            ( gspc::util::read_file (proc() / std::to_string (pid) / "cmdline")
            , '\0'
            )
          )
      {}

      std::list<procfs::entry> entries()
      {
        std::list<procfs::entry> es;

        for ( std::filesystem::directory_iterator de (proc())
            ; de != std::filesystem::directory_iterator()
            ; ++de
            )
        {
          if (std::filesystem::is_directory (de->status()))
          {
            std::string const name (de->path().filename().string());

            if ( !name.empty()
               && std::all_of ( name.begin(), name.end()
                              , [] (unsigned char c) { return std::isdigit (c); }
                              )
               )
            {
              try
              {
                es.emplace_back (name);
              }
              catch (...)
              {
                // ignore, the process went away
                std::ignore = std::current_exception();
              }
            }
          }
        }

        return es;
      }
    }
