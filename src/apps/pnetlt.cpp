// mirko.rahn@itwm.fraunhofer.de

#include <we/type/transition.hpp>

//! \todo eliminate this include
#include <we/type/net.hpp>
#include <we/mgmt/type/activity.hpp>

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include <fhg/revision.hpp>

#include <boost/range/adaptor/map.hpp>
#include <boost/foreach.hpp>

namespace
{
  class list_transition_name : public boost::static_visitor<void>
  {
  public:
    list_transition_name (std::ostream& os, const std::string& name)
      : _os (os)
      , _name (name)
    {
      _os << _name << std::endl;
    }

    void operator() (const we::type::expression_t&) const
    {}
    void operator() (const we::type::module_call_t&) const
    {}
    void operator() (const petri_net::net& n) const
    {
      BOOST_FOREACH ( const we::type::transition_t& t
                    , n.transitions() | boost::adaptors::map_values
                    )
      {
        boost::apply_visitor
          ( list_transition_name (_os, _name + "." + t.name())
          , t.data()
          );
      }
    }

  private:
    std::ostream& _os;
    const std::string& _name;
  };

  void tl (std::ostream& os, const we::mgmt::type::activity_t& a)
  {
    boost::apply_visitor ( list_transition_name (os, a.transition().name())
                         , a.transition().data()
                         );
  }
}

int
main (int argc, char** argv)
try
{
  namespace po = boost::program_options;

  std::string input;
  std::string output;

  po::options_description desc ("General");

  desc.add_options()
    ( "help,h", "this message")
    ( "version,V", "print version information")
    ( "input,i"
    , po::value<std::string>(&input)->default_value ("-")
    , "input file name, - for stdin, first positional parameter"
    )
    ( "output,o"
    , po::value<std::string>(&output)->default_value ("-")
    , "output file name, - for stdout, second positional parameter"
    )
    ;

  po::positional_options_description p;
  p.add ("input", 1).add ("output", 2);

  po::variables_map vm;
  po::store ( po::command_line_parser (argc, argv)
            . options (desc).positional (p).run()
            , vm
            );
  po::notify(vm);

  if (vm.count ("help"))
    {
      std::cout << argv[0] << ": list transition names" << std::endl
                << desc << std::endl
        ;
      return EXIT_SUCCESS;
    }

  if (vm.count ("version"))
    {
      std::cout << fhg::project_info ("pnet2dot");

      return EXIT_SUCCESS;
    }

  const we::mgmt::type::activity_t act
    ( input == "-"
    ? we::mgmt::type::activity_t (std::cin)
    : we::mgmt::type::activity_t (boost::filesystem::path (input))
    );

  if (output == "-")
  {
    tl (std::cout, act);
  }
  else
  {
    std::ofstream os (output.c_str());

    if (!os)
    {
      throw std::runtime_error ("failed to open " + output + " for writing");
    }

    tl (os, act);
  }

  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
