#include "config_cmd.hpp"
#include "config.hpp"
#include "system.hpp"

#include <sysexits.h>

#include <boost/foreach.hpp>

namespace gspc
{
  namespace ctl
  {
    namespace cmd
    {
      int config_cmd ( std::vector<std::string> const & argv
                     , std::istream &inp
                     , std::ostream &out
                     , std::ostream &err
                     )
      {
        enum config_mode_t
        {
          MGET
        , MGETALL
        , MUNSET
        , MUNSETALL
        , MSET
        , MADD
        , MEDIT
        , MLIST
        , MREPLACE
        , MNONE
        };

        std::string file;
        config_mode_t mode = MNONE;

        size_t i = 1;
        while (i < argv.size ())
        {
          const std::string arg = argv [i];

          if (arg == "--help" || arg == "-h")
          {
            ++i;
            err << "usage: config [<file-option>] [type] command..."  << std::endl
                <<                                                       std::endl
                << "   file-options"                                  << std::endl
                << "       --user : selects the users global config"  << std::endl
                << "       --system : selects the system config"      << std::endl
                << "       --site : selects the site config"          << std::endl
                << "       -f | --file file : use this file"          << std::endl
                <<                                                       std::endl
              /*
                << "   types"                                         << std::endl
                << "       --int : value is an integer"               << std::endl
                << "       --bool : value is a boolean"               << std::endl
                << "       --string : value is a string"              << std::endl
                <<                                                       std::endl
              */
                << "   available commands"                            << std::endl
                << "       --add name value"                          << std::endl
                << "       --get name [regex]"                        << std::endl
                << "       --get-all name [regex]"                    << std::endl
                << "       --unset name [regex]"                      << std::endl
                << "       --replace name value [value-regex]"        << std::endl
                << "       --list"                                    << std::endl
                << "       -e | --edit"                               << std::endl
              ;

            return 0;
          }
          else if (arg == "--user")
          {
            file = gspc::ctl::user_config_file ();
            ++i;
          }
          else if (arg == "--system")
          {
            file = gspc::ctl::system_config_file ();
            ++i;
          }
          else if (arg == "--site")
          {
            file = gspc::ctl::site_config_file ();
            ++i;
          }
          else if (arg == "-f" || arg == "--file")
          {
            ++i;

            if (i < argv.size ())
            {
              file = argv [i];
            }
            else
            {
              err << "config: missing argument to '--file'" << std::endl;
              return EX_USAGE;
            }
          }
          else if (arg == "-e" || arg == "--edit")
          {
            mode = MEDIT;
            ++i;
          }
          else if (arg == "--get")
          {
            mode = MGET;
            ++i;
            break;
          }
          else if (arg == "--get-all")
          {
            mode = MGETALL;
            ++i;
          }
          else if (arg == "--set")
          {
            mode = MSET;
            ++i;

            if ((i + 1) < argv.size ())
            {
              break;
            }
            else
            {
              err << "config: missing argument(s) to '--set'" << std::endl;
              return EX_USAGE;
            }
          }
          else if (arg == "--add")
          {
            mode = MADD;
            ++i;

            if ((i + 1) < argv.size ())
            {
              break;
            }
            else
            {
              err << "config: missing argument(s) to '--add'" << std::endl;
              return EX_USAGE;
            }
          }
          else if (arg == "--unset")
          {
            mode = MUNSET;
            ++i;

            if (i < argv.size ())
            {
              break;
            }
            else
            {
              err << "config: missing argument to '--unset'" << std::endl;
              return EX_USAGE;
            }
          }
          else if (arg == "--unset-all")
          {
            mode = MUNSETALL;
            ++i;
          }
          else if (arg == "--list")
          {
            mode = MLIST;
            ++i;
          }
          else if (arg == "--replace")
          {
            mode = MREPLACE;
            ++i;

            if ((i+1) < argv.size ())
            {
              break;
            }
            else
            {
              err << "config: missing argument to '--replace'" << std::endl;
              return EX_USAGE;
            }
          }
          else
          {
            break;
          }
        }

        if (mode == MNONE)
        {
          mode = MGET;
        }

        switch (mode)
        {
        case MGET:
          {
            if (i == argv.size ())
            {
              err << "usage: config [<file-option>] --get key [value-regex]"
                  << std::endl;
              return EX_USAGE;
            }

            config_t cfg =
              file.empty () ? config_read () : config_read_safe (file);
            std::vector<std::string> matches =
              config_get_all ( cfg
                             , argv [i]
                             , (i+1) < argv.size () ? argv [i+1] : ""
                             );
            if (matches.empty ())
            {
              return EXIT_FAILURE;
            }
            else
            {
              std::cout << matches.back () << std::endl;
            }
            break;
          }
        case MGETALL:
          {
            if (i == argv.size ())
            {
              err << "usage: config [<file-option>] --get-all key [value-regex]"
                  << std::endl;
              return EX_USAGE;
            }

            config_t cfg =
              file.empty () ? config_read () : config_read_safe (file);
            std::vector<std::string> matches =
              config_get_all ( cfg
                             , argv [i]
                             , (i+1) < argv.size () ? argv [i+1] : ""
                             );
            if (matches.empty ())
            {
              return EXIT_FAILURE;
            }
            else
            {
              BOOST_FOREACH (std::string const &m, matches)
              {
                std::cout << m << std::endl;
              }
            }
            break;
          }
        case MEDIT:
          {
            char buf [4096];
            snprintf (buf, sizeof(buf), "editor %s", file.c_str ());
            return system (buf);
          }
          break;
        case MLIST:
          {
            config_t cfg =
              file.empty () ? config_read () : config_read_safe (file);
            typedef std::vector<std::pair<std::string, std::string> > flat_list_t;
            flat_list_t flat_list = config_list (cfg);
            for (flat_list_t::const_iterator it = flat_list.begin () ; it != flat_list.end () ; ++it)
            {
              out << it->first << "=" << it->second << std::endl;
            }
          }
          return 0;
        case MADD:
          {
            if ((argv.size () - i) < 2)
            {
              err << "usage: config [<file-option>] --add key value"
                  << std::endl;
              return EX_USAGE;
            }

            if (file.empty ())
              file = user_config_file ();

            config_t cfg = config_read_safe (file);
            try
            {
              config_add (cfg, argv [i], argv [i+1]);
              config_write (cfg, file);
            }
            catch (std::exception const &ex)
            {
              std::cerr << "add: failed: " << ex.what () << std::endl;
              return EXIT_FAILURE;
            }
          }
          return 0;
        case MUNSET:
          {
            if ((argv.size () - i) < 1)
            {
              err << "usage: config [<file-option>] --unset key [value-regex]"
                  << std::endl;
              return EX_USAGE;
            }

            if (file.empty ())
              file = user_config_file ();

            config_t cfg = config_read_safe (file);
            config_unset ( cfg
                         , argv [i]
                         , ((i+1) < argv.size ()) ? argv [i+1] : ""
                         );
            config_write (cfg, file);
          }
          return 0;
        case MREPLACE:
          {
            if ((argv.size () - i) < 2)
            {
              err << "usage: config [<file-option>] --replace key value [value-regex]"
                  << std::endl;
              return EX_USAGE;
            }

            if (file.empty ())
              file = user_config_file ();

            config_t cfg = config_read_safe (file);
            config_replace ( cfg
                           , argv [i]
                           , argv [i+1]
                           , ((i+2) < argv.size ()) ? argv [i+2] : ""
                           );
            config_write (cfg, file);
          }
          return 0;
        default:
          err << "mode " << mode << " not yet implemented" << std::endl;
          return EX_SOFTWARE;
        }

        return 0;
      }
    }
  }
}
