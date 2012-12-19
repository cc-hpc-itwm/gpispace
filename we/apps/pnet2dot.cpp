// mirko.rahn@itwm.fraunhofer.de

#include <we/net.hpp>
#include <we/util/codec.hpp>

#include <we/type/bits/transition/toDot.hpp>

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/function.hpp>

#include <fhg/util/starts_with.hpp>

#include <fhg/revision.hpp>

// ************************************************************************* //

namespace detail {
  template<typename Activity, typename Pred>
  void to_dot (std::ostream & os, const Activity & a, const Pred & pred)
  {
    we::type::dot::id_type id (0);

    we::type::dot::init (a.transition().prop());

    os << "digraph " << a.transition().name() << " {" << std::endl;
    os << "compound=true" << std::endl;
    os << "rankdir=LR" << std::endl;
    os << we::type::dot::to_dot (a.transition(), id, pred);
    os << "} /* " << a.transition().name() << " */" << std::endl;
  }
}

// ************************************************************************* //

template<typename T>
bool name_not_starts_with (const std::string & p, const T & x)
{
  return !fhg::util::starts_with (p, x.name());
}

template<typename T>
bool name_not_ends_with (const std::string & s, const T & x)
{
  return !fhg::util::ends_with (s, x.name());
}

// ************************************************************************* //

typedef std::vector<std::string> vec_type;

template<typename T>
bool all ( boost::function<bool (const std::string &, const T &)> f
         , const vec_type & ys
         , const T & x
         )
{
  bool all_okay (true);

  for ( vec_type::const_iterator y (ys.begin())
      ; y != ys.end() && all_okay
      ; ++y
      )
    {
      all_okay &= f (*y, x);
    }

  return all_okay;
}

template<typename T>
bool pred_and ( const boost::function<bool (const T &)> f
              , const boost::function<bool (const T &)> g
              , const T & x
              )
{
  return f(x) && g(x);
}

// ************************************************************************* //

namespace po = boost::program_options;

int
main (int argc, char ** argv)
{
  std::string input;
  std::string output;

  vec_type not_starts_with;
  vec_type not_ends_with;

  typedef we::type::dot::generic<we::type::transition_t> pred_t;

  we::type::dot::options<pred_t> options;

  po::options_description desc ("General");
  po::options_description show ("Show");
  po::options_description expand ("Expand");

#define BOOLVAL(x) po::value<bool>(&x)->default_value(x)->implicit_value(true)

  show.add_options ()
    ( "full-signatures"
    , BOOLVAL(options.full)
    , "whether or not to show full signatures"
    )
    ( "token"
    , BOOLVAL(options.show_token)
    , "whether or not to show the tokens on a place"
    )
    ( "signature"
    , BOOLVAL(options.show_signature)
    , "whether or not to show the place and port signatures"
    )
    ( "priority"
    , BOOLVAL(options.show_priority)
    , "whether or not to show the transition priority"
    )
    ( "intext"
    , BOOLVAL(options.show_intext)
    , "whether or not to show the transition internal/external flag"
    )
    ( "virtual"
    , BOOLVAL(options.show_virtual)
    , "whether or not to show the virtual flag"
    )
    ( "real"
    , BOOLVAL(options.show_real)
    , "whether or not to show the real places, associated with a place"
    )
    ( "tunnel-connection"
    , BOOLVAL(options.show_tunnel_connection)
    , "whether or not to show the tunnel connections"
    )
    ;

  expand.add_options ()
    ( "not-starts-with"
    , po::value<vec_type>(&not_starts_with)
    , "do not expand transitions that start with a certain prefix"
    )
    ( "not-ends-with"
    , po::value<vec_type>(&not_ends_with)
    , "do not expand transitions that end with a certain suffix"
    )
    ;

  desc.add_options()
    ( "help,h", "this message")
    ( "version,V", "print version information")
    ( "input,i"
    , po::value<std::string>(&input)->default_value("-")
    , "input file name, - for stdin, first positional parameter"
    )
    ( "output,o"
    , po::value<std::string>(&output)->default_value("-")
    , "output file name, - for stdout, second positional parameter"
    )
    ;

  desc.add (show).add (expand);

#undef BOOLVAL

  po::positional_options_description p;
  p.add("input", 1).add("output", 2);

  po::variables_map vm;
  po::store( po::command_line_parser(argc, argv)
           . options(desc).positional(p).run()
           , vm
           );
  po::notify(vm);

  if (vm.count("help"))
    {
      std::cout << argv[0] << ": convert to graphviz format" << std::endl;

      std::cout << desc << std::endl;
      return EXIT_SUCCESS;
    }

  if (vm.count("version"))
    {
      std::cout << fhg::project_info();

      return EXIT_SUCCESS;
    }

  if (input == "-")
    {
      input = "/dev/stdin";
    }

  if (output == "-")
    {
      output = "/dev/stdout";
    }

  boost::function<bool (const we::type::transition_t &)> not_starts
    ( boost::bind ( all<we::type::transition_t>
                  , name_not_starts_with<we::type::transition_t>
                  , not_starts_with
                  , _1
                  )
    );

  boost::function<bool (const we::type::transition_t &)> not_ends
    ( boost::bind ( all<we::type::transition_t>
                  , name_not_ends_with<we::type::transition_t>
                  , not_ends_with
                  , _1
                  )
    );

  options.predicate = pred_t ( boost::bind ( pred_and<we::type::transition_t>
                                           , not_starts
                                           , not_ends
                                           , _1
                                           )
                             );

  we::mgmt::type::activity_t act;

  {
    std::ifstream stream (input.c_str());

    if (!stream)
      {
        throw std::runtime_error ("failed to open " + input + " for reading");
      }

    we::util::codec::decode (stream, act);
  }

  {
    std::ofstream stream (output.c_str());

    if (!stream)
      {
        throw std::runtime_error ("failed to open " + output + " for writing");
      }

    detail::to_dot (stream, act, options);
  }

  return EXIT_SUCCESS;
}
