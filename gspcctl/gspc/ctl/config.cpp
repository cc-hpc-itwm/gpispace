#include "config.hpp"
#include "system.hpp"

#include <sysexits.h>

#include <fstream>

#include <boost/foreach.hpp>
#include <boost/system/error_code.hpp>

#include <json_spirit_reader_template.h>
#include <json_spirit_writer_template.h>

namespace gspc
{
  namespace ctl
  {
    namespace error
    {
      enum config_errors
        {
          config_no_section
        , config_no_name
        , config_no_such_key
        , config_invalid_key
        };
    }

    namespace detail
    {
      class config_category : public boost::system::error_category
      {
      public:
        const char *name () const
        {
          return "gspc.config";
        }

        std::string message (int value) const
        {
          switch (value)
          {
          case error::config_no_section:
            return "key did not contain a section";
          case error::config_no_name:
            return "key did not contain a name";
          case error::config_no_such_key:
            return "key not found";
          case error::config_invalid_key:
            return "invalid key";
          default:
            return "gspc.config error";
          }
        }
      };
    }

    inline const boost::system::error_category & get_config_category ()
    {
      static detail::config_category cat;
      return cat;
    }
  }
}

namespace boost
{
  namespace system
  {
    template <> struct is_error_code_enum<gspc::ctl::error::config_errors>
    {
      static const bool value = true;
    };
  }
}

namespace gspc
{
  namespace ctl
  {
    namespace error
    {
      inline boost::system::error_code make_error_code (config_errors e)
      {
        return boost::system::error_code (static_cast<int>(e), get_config_category ());
      }
    }
  }
}

namespace gspc
{
  namespace ctl
  {
    config_t config_read ()
    {
      return config_read_user ();
    }

    config_t config_read_site ()
    {
      try
      {
        return config_read (site_config_file ());
      }
      catch (std::runtime_error const &)
      {
        return json_spirit::Object ();
      }
    }

    config_t config_read_system ()
    {
      try
      {
        return config_read (system_config_file ());
      }
      catch (std::runtime_error const &)
      {
        return json_spirit::Object ();
      }
    }

    config_t config_read_user ()
    {
      try
      {
        return config_read (user_config_file ());
      }
      catch (std::runtime_error const &)
      {
        return json_spirit::Object ();
      }
    }

    config_t config_read (std::istream &is)
    {
      config_t cfg;
      const bool success = json_spirit::read_stream (is, cfg);
      if (not success)
      {
        throw std::runtime_error ("could not parse");
      }
      return cfg;
    }

    config_t config_read (std::string const &file)
    {
      std::ifstream is (file.c_str ());
      try
      {
        return config_read (is);
      }
      catch (std::runtime_error const &e)
      {
        throw std::runtime_error (std::string (e.what ()) + ": " + file);
      }
    }

    void config_write (config_t const &cfg, std::ostream & os)
    {
      json_spirit::write_stream (cfg, os, json_spirit::pretty_print);
    }

    void config_write (config_t const &cfg, std::string const &fname)
    {
      std::ofstream os (fname.c_str ());
      config_write (cfg, os);
    }

    static int s_split_key ( std::string const &key
                           , std::vector<std::string> &path
                           )
    {
      if (key.find ("=") != std::string::npos)
        return error::config_invalid_key;

      std::string::size_type left = key.find (".");
      std::string::size_type right = key.rfind (".");

      if (left == 0)
      {
        return error::config_no_section;
      }
      if (left == std::string::npos || right == (key.size ()-1))
      {
        return error::config_no_name;
      }

      path.push_back (key.substr (0, left));
      if (left - right > 0)
        path.push_back (key.substr (left+1, right - left - 1));
      path.push_back (key.substr (right+1));

      return 0;
    }

    static void s_flatten ( json_spirit::Value const &val
                          , std::vector<std::pair<std::string, std::string> >& l
                          , std::string const &key = ""
                          )
    {
      if (val.is_null ())
        return;

      if (val.type () == json_spirit::obj_type)
      {
        json_spirit::Object const & next = val.get_obj ();
        json_spirit::Object::const_iterator it = next.begin ();
        const json_spirit::Object::const_iterator end = next.end ();

        while (it != end)
        {
          std::string child =
            key.empty () ? it->name_ : (key + "." + it->name_);
          s_flatten (it->value_, l, child);
          ++it;
        }
      }
      else if (val.type () == json_spirit::array_type)
      {
        json_spirit::Array const & next = val.get_array ();
        json_spirit::Array::const_iterator it = next.begin ();
        const json_spirit::Array::const_iterator end = next.end ();

        while (it != end)
        {
          s_flatten (*it, l, key);
          ++it;
        }
      }
      else
      {
        l.push_back (std::make_pair (key, val.get_str ()));
      }
    }

    std::vector<std::string> config_get_all ( config_t const &cfg
                                            , std::string const &key
                                            )
    {
      std::vector<std::string> result;

      typedef std::vector<std::pair<std::string, std::string> > flat_list_t;
      flat_list_t flat_list;
      s_flatten (cfg, flat_list);
      for (flat_list_t::const_iterator it = flat_list.begin () ; it != flat_list.end () ; ++it)
      {
        if (it->first == key)
          result.push_back (it->second);
      }

      return result;
    }

    std::string config_get_str (config_t const &cfg, std::string const &key)
    {
      std::vector<std::string> v = config_get_all (cfg, key);
      if (v.empty ())
      {
        throw make_error_code (error::config_no_such_key);
      }
      else
      {
        return v.back ();
      }
    }

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
      , MNONE
      };

      std::string file (gspc::ctl::user_config_file ());
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
            err << "usage: config [<file-option>] [type] command..."
                << std::endl;
            return EX_USAGE;
          }

          config_t cfg = config_read (file);
          std::vector<std::string> matches = config_get_all (cfg, argv [i]);
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
            err << "usage: config [<file-option>] [type] command..."
                << std::endl;
            return EX_USAGE;
          }

          config_t cfg = config_read (file);
          std::vector<std::string> matches = config_get_all (cfg, argv [i]);
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
          config_t cfg = config_read (file);
          typedef std::vector<std::pair<std::string, std::string> > flat_list_t;
          flat_list_t flat_list;
          s_flatten (cfg, flat_list);
          for (flat_list_t::const_iterator it = flat_list.begin () ; it != flat_list.end () ; ++it)
          {
            out << it->first << "=" << it->second << std::endl;
          }
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
