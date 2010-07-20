// mirko.rahn@itwm.fraunhofer.de

#include <we/we.hpp>

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/function.hpp>

// ************************************************************************* //

template<typename IT>
static bool
generic_starts_with ( IT pos_p, const IT end_p
                    , IT pos_x, const IT end_x
                    )
{
  while (pos_p != end_p && pos_x != end_x)
    {
      if (*pos_p != *pos_x)
        {
          return false;
        }

      ++pos_p;
      ++pos_x;
    }

  if (pos_p == end_p)
    {
      return true;
    }

  return false;
}

static bool
starts_with (const std::string & p, const std::string & x)
{
  return generic_starts_with (p.begin(), p.end(), x.begin(), x.end());
}

static bool
ends_with (const std::string & s, const std::string & x)
{
  return generic_starts_with (s.rbegin(), s.rend(), x.rbegin(), x.rend());
}

// ************************************************************************* //

template<typename T>
bool name_not_starts_with (const std::string & p, const T & x)
{
  return !starts_with (p, x.name());
}

template<typename T>
bool name_not_ends_with (const std::string & s, const T & x)
{
  return !ends_with (s, x.name());
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
    ("help", "this message")
    ( "input"
    , po::value<std::string>(&input)->default_value("-")
    , "input file name, - for stdin"
    )
    ( "output"
    , po::value<std::string>(&output)->default_value("-")
    , "output file name, - for stdout"
    )
    ( "full-signatures"
    , po::value<bool>(&options.full)->default_value(options.full)
    , "whether or not to show full signatures"
    )
    ( "show-token"
    , po::value<bool>(&options.show_token)->default_value(options.show_token)
    , "whether or not show the tokens on a place"
    )
    ( "show-capacity"
    , po::value<bool>(&options.show_capacity)->default_value(options.show_capacity)
    , "whether or not show the place capacities"
    )
    ( "show-signature"
    , po::value<bool>(&options.show_signature)->default_value(options.show_signature)
    , "whether or not show the place and port signatures"
    )
    ( "show-priority"
    , po::value<bool>(&options.show_priority)->default_value(options.show_priority)
    , "whether or not show the transition priority"
    )
    ( "show-intext"
    , po::value<bool>(&options.show_intext)->default_value(options.show_intext)
    , "whether or not show the transition internal/external falg"
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

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
    {
      std::cout << desc << std::endl;
      return EXIT_SUCCESS;
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

  if (input == "-")
    {
      we::util::text_codec::decode (std::cin, act);
    }
  else
    {
      std::ifstream stream (input.c_str());

      we::util::text_codec::decode (stream, act);
    }

  act.dot (std::cout, options);
  
  return EXIT_SUCCESS;
}
