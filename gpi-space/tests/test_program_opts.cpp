#define BOOST_TEST_MODULE ProgramOptsTest
#include <boost/test/unit_test.hpp>

#include <boost/program_options.hpp>
#include <boost/foreach.hpp>

#include <string>
#include <vector>

namespace po = boost::program_options;

BOOST_AUTO_TEST_CASE ( test_string_vector_pos_args )
{
  typedef std::vector<std::string> str_list_t;
  str_list_t args;

  po::options_description desc ("");
  desc.add_options ()
    ("str,s", po::value<str_list_t>(&args), "positional arguments")
    ;

  po::positional_options_description pos_opts;
  pos_opts.add ("str", -1);

  std::vector<std::string> av;
  av.push_back ("program");
  av.push_back ("--str"); av.push_back ("s1");
  av.push_back ("-s"); av.push_back ("s2");
  av.push_back ("s3");
  av.push_back ("s4");

  po::variables_map vm;

  po::store (po::command_line_parser (av)
            .options (desc)
            .positional (pos_opts)
            .run ()
            , vm
            );
  po::notify (vm);

  str_list_t const & strs = vm["str"].as<str_list_t>();
  {
    int i=0;
    BOOST_FOREACH (std::string const &s, strs)
    {
      BOOST_MESSAGE ("str [" << i++ << "] := " << s);
    }
  }
  BOOST_WARN_EQUAL (strs.size (), 4u);

  if (args.front () == av.front ())
    args.erase (args.begin ());

  {
    int i=0;
    BOOST_FOREACH (std::string const &s, args)
    {
      BOOST_MESSAGE ("str [" << i++ << "] := " << s);
    }
  }
  BOOST_REQUIRE_EQUAL (args.size (), 4u);
}
