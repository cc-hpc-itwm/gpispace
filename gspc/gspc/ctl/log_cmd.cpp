#include "log_cmd.hpp"
#include "system.hpp"

#include <sysexits.h>
#include <sstream>
#include <boost/lexical_cast.hpp>

#include <fhglog/fhglog.hpp>

namespace gspc
{
  namespace ctl
  {
    namespace cmd
    {
      namespace
      {
        struct fhglog_initializer
        {
          fhglog_initializer ()
          {
            FHGLOG_SETUP ();
          }
        };
      }

      int log_cmd ( std::vector<std::string> const & argv
                  , std::istream &inp
                  , std::ostream &out
                  , std::ostream &err
                  )
      {
        static fhglog_initializer _;

        size_t i = 1;
        std::string level ("DEF");
        std::string tag ("gspc");
        std::string file ("(STDIN)");
        std::string function ("(main)");
        int line = 0;

        while (i < argv.size ())
        {
          const std::string arg = argv [i];

          if (arg == "--help" || arg == "-h")
          {
            ++i;
            err << "usage: log [options] [--] message..."           << std::endl
                <<                                                     std::endl
                << "  options:"                                     << std::endl
                << "    --level <level>"                            << std::endl
                << "    --tag <tag>"                                << std::endl
                << "    --file <file>"                              << std::endl
                << "    --line <line>"                              << std::endl
                << "    --function <f>"                             << std::endl
                <<                                                     std::endl
              ;

            return 0;
          }
          else if (arg == "--level")
          {
            ++i;
            if (i == argv.size ())
            {
              std::cerr << "log: missing argument to --tag" << std::endl;
              return EXIT_FAILURE;
            }

            level = argv [i];
            ++i;
          }
          else if (arg == "--tag")
          {
            ++i;
            if (i == argv.size ())
            {
              std::cerr << "log: missing argument to --tag" << std::endl;
              return EXIT_FAILURE;
            }

            tag = argv [i];
            ++i;
          }
          else if (arg == "--file")
          {
            ++i;
            if (i == argv.size ())
            {
              std::cerr << "log: missing argument to --file" << std::endl;
              return EXIT_FAILURE;
            }

            file = argv [i];
            ++i;
          }
          else if (arg == "--line")
          {
            ++i;
            if (i == argv.size ())
            {
              std::cerr << "log: missing argument to --line" << std::endl;
              return EXIT_FAILURE;
            }

            line = boost::lexical_cast<int>(argv [i]);
            ++i;
          }
          else if (arg == "--function")
          {
            ++i;
            if (i == argv.size ())
            {
              std::cerr << "log: missing argument to --function" << std::endl;
              return EXIT_FAILURE;
            }

            function = argv [i];
            ++i;
          }
          else if (arg == "--")
          {
            ++i;
            break;
          }
          else
          {
            break;
          }
        }

        std::stringstream sstr;
        if (i < argv.size ())
        {
          sstr << argv [i];
          ++i;
        }
        while (i < argv.size ())
        {
          sstr << " " << argv [i];
          ++i;
        }

        fhg::log::Logger::get ("system")->log
          ( fhg::log::LogEvent ( fhg::log::from_string (level)
                               , file
                               , function
                               , line
                               , sstr.str().c_str()
                               , std::vector<std::string> (1, tag)
                               )
          );

        return 0;
      }
    }
  }
}
