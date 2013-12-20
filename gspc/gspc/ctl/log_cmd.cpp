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
        int level = -1;
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
            err << "usage: log [options] [--] message..."             << std::endl
                <<                                                       std::endl
                << "   options:"                                      << std::endl
                << "       --LEVEL : trace, debug, info, warn, error" << std::endl
                << "       --tag <tag> : how to tag the log event"    << std::endl
                <<                                                       std::endl
              ;

            return 0;
          }
          else if (arg == "--trace")
          {
            ++i;
            level = 0;
          }
          else if (arg == "--debug")
          {
            ++i;
            level = 1;
          }
          else if (arg == "--info")
          {
            ++i;
            level = 2;
          }
          else if (arg == "--warn")
          {
            ++i;
            level = 3;
          }
          else if (arg == "--error")
          {
            ++i;
            level = 4;
          }
          else if (arg == "--fatal")
          {
            ++i;
            level = 5;
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


        if (  level < fhg::log::LogLevel::MIN_LEVEL
           || level > fhg::log::LogLevel::MAX_LEVEL
           )
        {
          throw std::runtime_error ("STRANGE log level");
        }

        fhg::log::LogEvent evt ( fhg::log::LogLevel ((fhg::log::LogLevel::Level) level)
                               , file
                               , function
                               , line
                               , sstr.str().c_str()
                               , fhg::log::LogEvent::tags_type (1, tag)
                               );

        fhg::log::Logger::get ("system")->log (evt);

        return 0;
      }
    }
  }
}
