// mirko.rahn@itwm.fraunhofer.de

#include <we/we.hpp>

#include <we/type/bits/transition/toDot.hpp>

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/function.hpp>

#include <fhg/util/starts_with.hpp>

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

  typedef we::type::dot::generic<we::transition_t> pred_t;

  we::type::dot::options<pred_t> options;

  po::options_description desc("options");

  desc.add_options()
    ( "help,h", "this message")
    ( "input,i"
    , po::value<std::string>(&input)->default_value("-")
    , "input file name, - for stdin"
    )
    ( "output,o"
    , po::value<std::string>(&output)->default_value("-")
    , "output file name, - for stdout"
    )
    ( "full-signatures"
    , po::value<bool>(&options.full)->default_value(options.full)
    , "whether or not to show full signatures"
    )
    ( "show-token"
    , po::value<bool>(&options.show_token)->default_value(options.show_token)
    , "whether or not to show the tokens on a place"
    )
    ( "show-signature"
    , po::value<bool>(&options.show_signature)->default_value(options.show_signature)
    , "whether or not to show the place and port signatures"
    )
    ( "show-priority"
    , po::value<bool>(&options.show_priority)->default_value(options.show_priority)
    , "whether or not to show the transition priority"
    )
    ( "show-intext"
    , po::value<bool>(&options.show_intext)->default_value(options.show_intext)
    , "whether or not to show the transition internal/external flag"
    )
    ( "show-virtual"
    , po::value<bool>(&options.show_virtual)->default_value(options.show_virtual)
    , "whether or not to show the virtual flag"
    )
    ( "show-real"
    , po::value<bool>(&options.show_real)->default_value(options.show_real)
    , "whether or not to show the real places, associated with a place"
    )
    ( "show-tunnel-connection"
    , po::value<bool>(&options.show_tunnel_connection)->default_value(options.show_tunnel_connection)
    , "whether or not to show the tunnel connections"
    )
    ( "not-starts-with"
    , po::value<vec_type>(&not_starts_with)
    , "do not expand transitions that start with a certain prefix"
    )
    ( "not-ends-with"
    , po::value<vec_type>(&not_ends_with)
    , "do not expand transitions that end with a certain suffix"
    )
    ;

  po::positional_options_description p;
  p.add("input", -1);

  po::variables_map vm;
  po::store( po::command_line_parser(argc, argv)
           . options(desc).positional(p).run()
           , vm
           );
  po::notify(vm);

  if (vm.count("help"))
    {
      std::cout << desc << std::endl;
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

  boost::function<bool (const we::transition_t &)> not_starts
    ( boost::bind ( all<we::transition_t>
                  , name_not_starts_with<we::transition_t>
                  , not_starts_with
                  , _1
                  )
    );

  boost::function<bool (const we::transition_t &)> not_ends
    ( boost::bind ( all<we::transition_t>
                  , name_not_ends_with<we::transition_t>
                  , not_ends_with
                  , _1
                  )
    );

  options.predicate = pred_t ( boost::bind ( pred_and<we::transition_t>
                                           , not_starts
                                           , not_ends
                                           , _1
                                           )
                             );

  we::activity_t act;

  {
    std::ifstream stream (input.c_str());

    if (!stream)
      {
        throw std::runtime_error ("failed to open " + input + " for reading");
      }

    we::util::text_codec::decode (stream, act);
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
