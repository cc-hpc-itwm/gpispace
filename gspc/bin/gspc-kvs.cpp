#include <fhg/util/program_info.h>
#include <fhg/revision.hpp>

#include <unistd.h>   // exit
#include <errno.h>    // errno
#include <string.h>   // strerror
#include <sysexits.h> // exit codes
#include <stdlib.h>   // system

#include <string>
#include <iostream>

#include <boost/foreach.hpp>

#include <fhg/util/read.hpp>
#include <we/type/value.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <gspc/kvs.hpp>

static void long_usage (int lvl)
{
  std::cerr
    << "usage: gspc-kvs [options] [--] [command [args...]]"         << std::endl
    << ""                                                           << std::endl
    << "   -h|--help              print this help"                  << std::endl
    << "   -v|--verbose           be more verbose"                  << std::endl
    << ""                                                           << std::endl
    << "   --url URL              were is the kvs daemon"           << std::endl
    << ""                                                           << std::endl
    << "   --put key value        put some value into the kvs"      << std::endl
    << "                          can be specified multiple times"  << std::endl
    << "   --get key              get some key from the kvs"        << std::endl
    << "   --get-regex regex      get matching entries from the kvs"<< std::endl
    << "   --del key              delete the key from the kvs"      << std::endl
    << "   --del-regex regex      delete matching keys"             << std::endl
    << ""                                                           << std::endl
    << "   --push key value       push a key to the given list"     << std::endl
    << "   --pop  key [timeout]   try to pop a value from the"      << std::endl
    << "                          given list"                       << std::endl
    << ""                                                           << std::endl
    << "   --inc key [delta]      atomically increment a value"     << std::endl
    << ""                                                           << std::endl
    << "   --set-ttl key ttl      set the ttl of a given key"       << std::endl
    << "   --set-ttl-regex regex ttl set the ttl of matching keys"  << std::endl
    ;
}

static void short_usage ();

int main (int argc, char *argv [], char *envp [])
{
  int i;
  int verbose = 0;
  int help = 0;
  int rc = 0;
  bool there_was_something_to_get = false;

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
    std::string arg = argv [i];

    if (arg.size () == 0 || arg [0] != '-')
      break;

    if (arg == "--")
    {
      ++i;
      break;
    }

    if (arg.size () > 1 && arg [0] == '-' && arg [1] != '-')
    {
        ++i;

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
          default:
            std::cerr << "gspc: invalid flag: " << *flag << std::endl;
            return EX_USAGE;
          }
          ++flag;
        }
    }
    else if (arg == "--help")
    {
      ++i;
      ++help;
    }
    else if (arg == "--verbose")
    {
      ++verbose;
      ++i;
    }
    else if (arg == "--url")
    {
      ++i;
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --url" << std::endl;
        return EX_USAGE;
      }

      try
      {
        kvs = gspc::kvs::create (argv [i]);
      }
      catch (std::exception const &ex)
      {
        std::cerr << "kvs: failed: " << ex.what () << std::endl;
        return EX_UNAVAILABLE;
      }

      i += 1;
    }
    else if (arg ==  "--put")
    {
      ++i;
      if ((i+1) >= argc)
      {
        std::cerr << "kvs: missing argument to --put" << std::endl;
        return EX_USAGE;
      }

      try
      {
        to_put.push_back (std::make_pair ( argv [i]
                                         , pnet::type::value::read (argv [i+1])
                                         )
                         );
      }
      catch (std::exception const &ex)
      {
        std::cerr << "kvs: invalid value: " << ex.what () << std::endl;
        return EX_DATAERR;
      }

      i += 2;
    }
    else if (arg == "--get")
    {
      ++i;
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --get" << std::endl;
        return EX_USAGE;
      }

      there_was_something_to_get = true;

      pnet::type::value::value_type val;
      rc = kvs->get (argv [i], val);
      if (0 == rc)
      {
        results.push_back (std::make_pair (argv [i], val));
      }

      i += 1;
    }
    else if (arg == "--get-regex")
    {
      ++i;
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --get-regex" << std::endl;
        return EX_USAGE;
      }

      there_was_something_to_get = true;

      key_value_list_t values;
      rc = kvs->get_regex (argv [i], values);
      if (0 == rc)
      {
        results.insert ( results.end ()
                       , values.begin ()
                       , values.end ()
                       );
      }
      else
      {
        std::cerr << "kvs: failed: " << strerror (-rc) << std::endl;
        return EX_TEMPFAIL;
      }

      i += 1;
    }
    else if (arg == "--del")
    {
      ++i;
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --del" << std::endl;
        return EX_USAGE;
      }

      rc = kvs->del (argv [i]);

      i += 1;
    }
    else if (arg == "--del-regex")
    {
      ++i;
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --del-regex" << std::endl;
        return EX_USAGE;
      }

      rc = kvs->del_regex (argv [i]);

      i += 1;
    }
    else if (arg == "--push")
    {
      ++i;
      if ((i+1) >= argc)
      {
        std::cerr << "kvs: missing argument to --push" << std::endl;
        return EX_USAGE;
      }

      gspc::kvs::api_t::key_type key = argv [i];

      try
      {
        rc = kvs->push ( key
                       , pnet::type::value::read (argv [i+1])
                       );
        if (rc != 0)
        {
          std::cerr << "kvs: push to '" << key << "' failed: " << strerror (-rc)
                    << std::endl;
          return EX_DATAERR;
        }
      }
      catch (std::exception const &ex)
      {
        std::cerr << "kvs: invalid value: " << ex.what () << std::endl;
        return EX_DATAERR;
      }

      i += 2;
    }
    else if (arg == "--pop")
    {
      ++i;
      if (i == argc)
      {
        std::cerr << "kvs: missing argument to --pop" << std::endl;
        return EX_USAGE;
      }

      gspc::kvs::api_t::key_type key = argv [i++];
      int timeout = -1;
      if (i < argc)
      {
        try
        {
          timeout = fhg::util::read<int>(argv [i++]);
        }
        catch (std::exception const &ex)
        {
          std::cerr << "kvs: invalid timeout: " << ex.what () << std::endl;
          return EX_USAGE;
        }
      }

      there_was_something_to_get = true;

      pnet::type::value::value_type val;
      rc = kvs->pop ( key
                    , val
                    , timeout
                    );
      if (rc == -ETIME || rc == -EAGAIN)
      {
        return EX_TEMPFAIL;
      }
      if (rc != 0)
      {
        std::cerr << "kvs: pop failed: " << strerror (-rc) << std::endl;
        return EX_DATAERR;
      }

      results.push_back (std::make_pair (key, val));
    }
    else
    {
      ++i;
      std::cerr << "gspc: invalid option: " << arg << std::endl;
      short_usage ();
      return EX_USAGE;
    }
  }

  if (help)
  {
    long_usage (help);
    return 0;
  }

  if (not kvs)
  {
    std::cerr << "kvs: url is missing!" << std::endl;
    return EX_USAGE;
  }

  if (to_put.size ())
  {
    rc = kvs->put (to_put);
    if (rc != 0)
    {
      std::cerr << "kvs: put failed: " << strerror (-rc) << std::endl;
      return EX_TEMPFAIL;
    }
  }

  BOOST_FOREACH (key_value_list_t::value_type const &kv, results)
  {
    std::cout << kv.first << "=" << pnet::type::value::show (kv.second) << std::endl;
  }

  return there_was_something_to_get ? results.empty () : 0;
}

void short_usage ()
{
  std::cerr << "usage: gspc [options] [--] [command [args...]]" << std::endl;
}
