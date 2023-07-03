// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/procfs.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/split.hpp>

#include <boost/filesystem.hpp>

#include <algorithm>
#include <cctype>

namespace fhg
{
  namespace util
  {
    namespace procfs
    {
      namespace
      {
        ::boost::filesystem::path proc()
        {
          return "/proc";
        }
      }

      entry::entry (pid_t pid)
        : _pid (pid)
        , _command_line
          ( fhg::util::split<std::string, std::string>
            ( fhg::util::read_file (proc() / std::to_string (pid) / "cmdline")
            , '\0'
            )
          )
      {}

      std::list<procfs::entry> entries()
      {
        std::list<procfs::entry> es;

        for ( ::boost::filesystem::directory_iterator de (proc())
            ; de != ::boost::filesystem::directory_iterator()
            ; ++de
            )
        {
          if (de->status().type() == ::boost::filesystem::directory_file)
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
              }
            }
          }
        }

        return es;
      }
    }
  }
}
