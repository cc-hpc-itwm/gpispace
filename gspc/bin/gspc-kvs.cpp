#include <unistd.h>   // exit
#include <errno.h>    // errno
#include <string.h>   // strerror
#include <sysexits.h> // exit codes
#include <stdlib.h>   // system

#include <iomanip>
#include <string>
#include <iostream>

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include <fhg/util/num.hpp>
#include <we/type/value.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <gspc/net/io.hpp>
#include <gspc/kvs/net_frontend.hpp>

static void long_usage (int)
{
  std::cerr
    << "usage: gspc-kvs [options] [--] [commands [args...]]"        << std::endl
    << ""                                                           << std::endl
    << "   -h|--help              print this help"                  << std::endl
    << "   -v|--verbose           be more verbose"                  << std::endl
    << "   --show-mask {push,pop,del,put}"                          << std::endl
    << ""                                                           << std::endl
    << "   --url URL              were is the kvs daemon"           << std::endl
    << "   --timeout <ms>         timeout to wait"                  << std::endl
    << "   -o|--omit-key          do not print the key of results"  << std::endl
    << "   -s|--string            drop quotes if value is a string" << std::endl
    << ""                                                           << std::endl
    << "available commands"                                         << std::endl
    << "------------------"                                         << std::endl
    << ""                                                           << std::endl
    << "   --and                  if the previous command was"      << std::endl
    << "                          successful, continue"             << std::endl
    << "   --or                   if the previous command was"      << std::endl
    << "                          unsuccessful, continue"           << std::endl
    << ""                                                           << std::endl
    << "    example:"                                               << std::endl
    << "       --try-pop <key> --or --wait <key> push --pop <key>"  << std::endl
    << ""                                                           << std::endl
    << "   --put key value...     put key-value pairs into the kvs" << std::endl
    << "   --get key              get some key from the kvs"        << std::endl
    << "   --get-regex regex      get matching entries from the kvs"<< std::endl
    << "   --del key              delete the key from the kvs"      << std::endl
    << "   --del-regex regex      delete matching keys"             << std::endl
    << ""                                                           << std::endl
    << "   --wait key mask        wait for the given key to change" << std::endl
    << "   --push key value       push a key to the given list"     << std::endl
    << "   --pop  key             pop a value from the given list"  << std::endl
    << "   --try-pop key          try to pop a value from the"      << std::endl
    << "                          given list"                       << std::endl
    << ""                                                           << std::endl
    << "   --inc key              atomically increment a value"     << std::endl
    << "   --dec key              atomically decrement a value"     << std::endl
    << ""                                                           << std::endl
    << "   --set-ttl key ttl      set the ttl of a given key"       << std::endl
    << "   --set-ttl-regex regex ttl set the ttl of matching keys"  << std::endl
    ;
}

static void short_usage ();
static int verbose = 0;

namespace {
  enum show_flags_t
    {
      OMIT_KEY       = 0x01
    , UNQUOTE_STRING = 0x02
    };

  struct show
  {
    show ( gspc::kvs::api_t::key_type const &key
         , gspc::kvs::api_t::value_type const &value
         , int flags
         )
      : key (key)
      , value (value)
      , flags (flags)
    {}

    gspc::kvs::api_t::key_type const &key;
    gspc::kvs::api_t::value_type const &value;
    int flags;
  };

  std::ostream & operator << (std::ostream &os, const show &s)
  {
    if (0 == (s.flags & OMIT_KEY))
    {
      os << s.key << "=";
    }
    if ((s.flags & UNQUOTE_STRING) && boost::get<std::string>(&s.value) != 0)
    {
      os << boost::get<std::string>(s.value);
    }
    else
    {
      os << pnet::type::value::show (s.value);
    }
    return os;
  }

  int log_error (std::string const &tag, int ec)
  {
    if (verbose && ec < 0)
    {
      std::cerr << tag << " failed [" << -ec << "]: "
                << strerror (-ec)
                << std::endl;
    }

    return ec;
  }
}

int main (int argc, char *argv [], char **)
{
  int i;
  int help = 0;
  int rc = 0;
  int timeout = -1;
  int show_flags = 0;

  typedef std::list<std::pair< gspc::kvs::api_t::key_type
                             , gspc::kvs::api_t::value_type
                             >
                   > key_value_list_t;

  boost::shared_ptr<gspc::kvs::api_t> kvs;
  std::string url;
  key_value_list_t to_put;
  key_value_list_t results;

  i = 1;
  while (i < argc)
  {
    std::string arg = argv [i++];

    if (arg == "--")
    {
      break;
    }

    if (arg.size () > 1 && arg [0] == '-' && arg [1] != '-')
    {
        std::string::iterator flag = arg.begin ()+1;
        while (flag != arg.end ())
        {
          switch (*flag)
          {
          case 'v':
            ++verbose;
            break;
          case 'h':
            ++help;
            break;
          case 'o':
            show_flags |= OMIT_KEY;
            break;
          case 's':
            show_flags |= UNQUOTE_STRING;
            break;
          default:
            std::cerr << "kvs: invalid flag: " << *flag << std::endl;
            return EX_USAGE;
          }
          ++flag;
        }
    }
    else if (arg == "--help")
    {
      ++help;
    }
    else if (arg == "--verbose")
    {
      ++verbose;
    }
    else if (arg == "--omit-key")
    {
      show_flags |= OMIT_KEY;
    }
    else if (arg == "--string")
    {
      show_flags |= UNQUOTE_STRING;
    }
    else if (arg == "--show-mask")
    {
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --show-mask" << std::endl;
        return EX_USAGE;
      }

      std::string mask (argv [i]);
      ++i;

      std::ios_base::fmtflags oldflags = std::cout.flags ();

      std::cout << std::hex << std::setfill ('0') << std::showbase;

      if      (mask == "push" || mask == "popable")
        std::cout << std::setw (2)
                  << (gspc::kvs::api_t::E_PUSH | gspc::kvs::api_t::E_POPABLE)
                  << std::endl
          ;
      else if (mask == "pop")
        std::cout << std::setw (2)
                  << gspc::kvs::api_t::E_POP << std::endl;
      else if (mask == "put")
        std::cout << std::setw (2)
                  << gspc::kvs::api_t::E_PUT << std::endl;
      else if (mask == "del")
        std::cout << std::setw (2)
                  << gspc::kvs::api_t::E_DEL << std::endl;
      else if (mask == "exist")
        std::cout << std::setw (2)
                  << gspc::kvs::api_t::E_EXIST << std::endl;
      else
      {
        std::cerr << "kvs: invalid event mask: " << mask << std::endl;
        std::cerr << "kvs: must be one of: push, popable, pop, exist, del, put"
                  << std::endl;
        return EX_DATAERR;
      }
      std::cout.flags (oldflags);

      return 0;
    }
    else if (arg == "--url")
    {
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --url" << std::endl;
        return EX_USAGE;
      }

      url = argv [i++];
    }
    else if (arg == "--timeout")
    {
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --timeout" << std::endl;
        return EX_USAGE;
      }

      try
      {
        timeout = fhg::util::read_int (argv [i++]);
      }
      catch (std::exception const &ex)
      {
        std::cerr << "kvs: invalid timeout: " << ex.what () << std::endl;
        return EX_USAGE;
      }
    }
    else
    {
      --i;
      break;
    }
  }

  if (help)
  {
    long_usage (help);
    return 0;
  }

  gspc::net::initializer _net_init;

  try
  {
    kvs.reset (new gspc::kvs::kvs_net_frontend_t (url, _net_init));
  }
  catch (std::exception const &ex)
  {
    std::cerr << "kvs: failed: " << ex.what () << std::endl;
    return EX_UNAVAILABLE;
  }

  if (not kvs)
  {
    std::cerr << "kvs: url is missing!" << std::endl;
    return EX_USAGE;
  }

  // parse commands
  rc = 0;

  while (i < argc)
  {
    std::string arg = argv [i++];

    if (arg == "--and")
    {
      if (rc != 0)
      {
        break;
      }
    }
    else if (arg == "--or")
    {
      if (rc == 0)
      {
        break;
      }
    }
    else if (arg ==  "--put")
    {
      std::list<std::pair< gspc::kvs::api_t::key_type
                         , gspc::kvs::api_t::value_type
                         >
               > kv_list;

      while (i < argc)
      {
        if (argv [i][0] == '-')
          break;

        gspc::kvs::api_t::key_type key (argv [i++]);

        if (i == argc)
        {
          std::cerr << "kvs: missing val to --put <key>" << std::endl;
          return EX_USAGE;
        }

        gspc::kvs::api_t::value_type val;
        try
        {
          val = pnet::type::value::read (argv [i++]);
        }
        catch (std::exception const &ex)
        {
          std::cerr << "kvs: invalid value: " << ex.what () << std::endl;
          return EX_DATAERR;
        }

        kv_list.push_back (std::make_pair (key, val));
      }

      if (kv_list.empty ())
      {
        std::cerr << "kvs: put: nothing to put!" << std::endl;
        return EX_USAGE;
      }

      rc = log_error ("put", kvs->put (kv_list));
    }
    else if (arg == "--get")
    {
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --get" << std::endl;
        return EX_USAGE;
      }

      gspc::kvs::api_t::key_type key (argv [i++]);
      pnet::type::value::value_type val;
      rc = log_error ("get", kvs->get (key, val));

      if (0 == rc)
      {
        std::cout << show (key, val, show_flags) << std::endl;
      }
    }
    else if (arg == "--get-regex")
    {
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --get-regex" << std::endl;
        return EX_USAGE;
      }

      std::string regex (argv [i++]);
      key_value_list_t values;

      rc = log_error ("get-regex", kvs->get_regex (regex, values));

      if (0 == rc)
      {
        if (values.size ())
        {
          BOOST_FOREACH (key_value_list_t::value_type const &kv, values)
          {
            std::cout << show (kv.first, kv.second, show_flags) << std::endl;
          }
        }
        else
        {
          rc = 1;
        }
      }
    }
    else if (arg == "--del")
    {
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --del" << std::endl;
        return EX_USAGE;
      }

      gspc::kvs::api_t::key_type key (argv [i++]);

      rc = log_error ("del", kvs->del (key));
    }
    else if (arg == "--del-regex")
    {
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --del-regex" << std::endl;
        return EX_USAGE;
      }

      std::string regex (argv [i++]);

      rc = log_error ("del-regex", kvs->del_regex (regex));
    }
    else if (arg == "--set-ttl")
    {
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --set-ttl" << std::endl;
        return EX_USAGE;
      }
      gspc::kvs::api_t::key_type key = argv [i++];

      int ttl;
      try
      {
        ttl = fhg::util::read_int(argv [i++]);
      }
      catch (std::exception const &ex)
      {
        std::cerr << "kvs: invalid ttl" << std::endl;
        return EX_USAGE;
      }

      rc = log_error ("set-ttl", kvs->set_ttl (key, ttl));
    }
    else if (arg == "--set-ttl-regex")
    {
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --set-ttl-regex" << std::endl;
        return EX_USAGE;
      }
      std::string regex (argv [i++]);

      int ttl;
      try
      {
        ttl = fhg::util::read_int(argv [i++]);
      }
      catch (std::exception const &ex)
      {
        std::cerr << "kvs: invalid ttl" << std::endl;
        return EX_USAGE;
      }

      rc = log_error ("set-ttl-regex", kvs->set_ttl_regex (regex, ttl));
    }
    else if (arg == "--push")
    {
      if (i == argc)
      {
        std::cerr << "kvs: missing key to --push" << std::endl;
        return EX_USAGE;
      }
      gspc::kvs::api_t::key_type key = argv [i++];

      if (i == argc)
      {
        std::cerr << "kvs: missing value to --push" << std::endl;
        return EX_USAGE;
      }

      gspc::kvs::api_t::value_type val;
      try
      {
        val = pnet::type::value::read (argv [i++]);
      }
      catch (std::exception const &ex)
      {
        std::cerr << "kvs: invalid value: " << ex.what () << std::endl;
        return EX_DATAERR;
      }

      rc = log_error ("push", kvs->push (key, val));
    }
    else if (arg == "--wait")
    {
      if (i == argc)
      {
        std::cerr << "kvs: missing key to --wait" << std::endl;
        return EX_USAGE;
      }
      gspc::kvs::api_t::key_type key = argv [i++];

      if (i == argc)
      {
        std::cerr << "kvs: missing event-mask to --wait" << std::endl;
        return EX_USAGE;
      }

      std::string mask_string (argv [i++]);

      int mask;
      try
      {
        mask = fhg::util::read_int(mask_string);
      }
      catch (std::exception const &ex)
      {
        if (mask_string == "push")
          mask = gspc::kvs::api_t::E_PUSH | gspc::kvs::api_t::E_POPABLE;
        else if (mask_string == "pop")
          mask = gspc::kvs::api_t::E_POP;
        else if (mask_string == "del")
          mask = gspc::kvs::api_t::E_DEL;
        else if (mask_string == "put")
          mask = gspc::kvs::api_t::E_PUT;
        else if (mask_string == "exist")
          mask = gspc::kvs::api_t::E_EXIST;
        else
        {
          std::cerr << "kvs: invalid event-mask: " << mask_string << std::endl;
          return EX_USAGE;
        }
      }

      rc = log_error ("wait", kvs->wait (key, mask, timeout));
      if (rc >= 0 && (rc & mask))
      {
        rc = 0;
      }
    }
    else if (arg == "--try-pop")
    {
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --try-pop" << std::endl;
        return EX_USAGE;
      }

      gspc::kvs::api_t::key_type key = argv [i++];
      pnet::type::value::value_type val;
      rc = log_error ("pop", kvs->try_pop (key, val));

      if (0 == rc)
      {
        std::cout << show (key, val, show_flags) << std::endl;
      }
    }
    else if (arg == "--pop")
    {
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --pop" << std::endl;
        return EX_USAGE;
      }

      gspc::kvs::api_t::key_type key = argv [i++];
      pnet::type::value::value_type val;
      rc = log_error ("pop", kvs->pop (key, val, timeout));

      if (0 == rc)
      {
        std::cout << show (key, val, show_flags) << std::endl;
      }
    }
    else if (arg == "--inc")
    {
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --inc" << std::endl;
        return EX_USAGE;
      }

      gspc::kvs::api_t::key_type key = argv [i++];
      int val;
      rc = log_error ("inc", kvs->counter_increment (key, val));

      if (0 == rc)
      {
        std::cout << show (key, val, show_flags) << std::endl;
      }
    }
    else if (arg == "--dec")
    {
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --dec" << std::endl;
        return EX_USAGE;
      }

      gspc::kvs::api_t::key_type key = argv [i++];
      int val;
      rc = log_error ("dec", kvs->counter_decrement (key, val));

      if (0 == rc)
      {
        std::cout << show (key, val, show_flags) << std::endl;
      }
    }
    else
    {
      std::cerr << "gspc: invalid option: " << arg << std::endl;
      short_usage ();
      return EX_USAGE;
    }
  }

  if (rc == -ETIME || rc == -EAGAIN)
  {
    return EX_TEMPFAIL;
  }
  else if (rc == -ECONNREFUSED)
  {
    return EX_UNAVAILABLE;
  }
  else if (rc == -EPERM)
  {
    return EX_NOPERM;
  }
  else if (rc == -ENOKEY)
  {
    return 1;
  }
  else if (rc < 0)
  {
    return EX_SOFTWARE;
  }
  else
  {
    return rc;
  }
}

void short_usage ()
{
  std::cerr << "usage: gspc [options] [--] [command [args...]]" << std::endl;
}
