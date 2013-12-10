// mirko.rahn@itwm.fraunhofer.de

#include <we/type/transition.hpp>

//! \todo eliminate this include
#include <we/type/net.hpp>
#include <we/mgmt/type/activity.hpp>

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include <fhg/revision.hpp>
#include <fhg/util/split.hpp>

#include <boost/range/adaptor/map.hpp>
#include <boost/foreach.hpp>

namespace
{
  typedef std::list<std::string> name_path_type;
  typedef std::list<we::type::transition_t*> transition_path_type;

  template<typename Container>
    class append
  {
  public:
    append (Container& container, const typename Container::value_type& value)
      : _container (container)
    {
      _container.push_back (value);
    }
    ~append()
    {
      _container.pop_back();
    }
    operator Container&() const
    {
      return _container;
    }

  private:
    Container& _container;
  };

  template<typename State>
  void visit
    ( we::type::transition_t&
    , State&
    , name_path_type&
    , transition_path_type&
    , const petri_net::transition_id_type& = petri_net::transition_id_invalid()
    );

  template<typename State>
  class list_transition_name : public boost::static_visitor<void>
  {
  public:
    list_transition_name ( transition_path_type& trs
                         , name_path_type& path
                         , State& state
                         )
      : _trs (trs)
      , _path (path)
      , _state (state)
    {
      _state (_path, _trs);
    }

    void operator() (we::type::expression_t&) const
    {}
    void operator() (we::type::module_call_t&) const
    {}
    void operator() (petri_net::net& n) const
    {
      n.modify_transitions
        (boost::bind (visit<State>, _2, _state, _path, _trs, _1));
    }

  private:
    transition_path_type& _trs;
    name_path_type& _path;
    State& _state;
  };

  template<typename State>
  void visit ( we::type::transition_t& tr
             , State& state
             , name_path_type& path
             , transition_path_type& trs
             , const petri_net::transition_id_type&
             )
  {
    boost::apply_visitor
      ( list_transition_name<State>
        ( append<transition_path_type > (trs, &tr)
        , append<name_path_type> (path, tr.name())
        , state
        )
      , tr.data()
      );
  }

  template<typename State>
  void start_visit ( we::type::transition_t& tr
                   , State& state
                   )
  {
    name_path_type path;
    transition_path_type trs;

    visit (tr, state, path, trs);
  }
}

namespace
{
  struct state
  {
    state (const name_path_type& needle, const we::type::requirement_t& req)
      : _needle (needle)
      , _requirement (req)
    {}

    const name_path_type& _needle;
    const we::type::requirement_t& _requirement;

    void operator() ( const name_path_type& path
                    , const transition_path_type& trs
                    )
    {
      if (_needle == path)
      {
        BOOST_FOREACH (we::type::transition_t* tr, trs)
        {
          tr->add_requirement (_requirement);
        }
      }
    }
  };

  void set_req ( const name_path_type& needle
               , const we::type::requirement_t& requirement
               , we::mgmt::type::activity_t& a
               )
  {
    state st (needle, requirement);
    start_visit<state> (a.transition(), st);
  }
}

int
main (int argc, char** argv)
try
{
  namespace po = boost::program_options;

  std::string input;
  std::string output;
  std::string needle_str;
  std::string req ("DEBUG");
  bool mandatory (true);

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
    ( "needle,n"
    , po::value<std::string> (&needle_str)
    , "transition name to find"
    )
    ( "requirement,r"
    , po::value<std::string> (&req)->default_value (req)
    , "the name of the capability to set"
    )
    ( "mandatory,m"
    , po::value<bool> (&mandatory)->default_value (mandatory)
    , "whether or not the requirement is mandatory"
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

  if (vm.count ("help") || !vm.count ("needle"))
    {
      std::cout << argv[0] << ": list transition names" << std::endl
                << desc << std::endl
        ;
      return EXIT_SUCCESS;
    }

  if (vm.count ("version"))
    {
      std::cout << fhg::project_info ("pnetsc");

      return EXIT_SUCCESS;
    }

  we::mgmt::type::activity_t act
    ( input == "-"
    ? we::mgmt::type::activity_t (std::cin)
    : we::mgmt::type::activity_t (boost::filesystem::path (input))
    );

  const name_path_type needle
    (fhg::util::split<std::string, name_path_type> (needle_str, '.'));
  const we::type::requirement_t requirement (req, mandatory);

  if (output == "-")
  {
    set_req (needle, requirement, act);

    std::cout << act;
  }
  else
  {
    std::ofstream os (output.c_str());

    if (!os)
    {
      throw std::runtime_error ("failed to open " + output + " for writing");
    }

    set_req (needle, requirement, act);

    os << act;
  }

  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
