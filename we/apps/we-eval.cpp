
#include <we/loader/loader.hpp>
#include <we/loader/module_call.hpp>
#include <we/mgmt/context.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include <stdint.h>
#include <fhglog/fhglog.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/getenv.hpp>

#include <we/mgmt/type/activity.hpp>
#include <we/util/codec.hpp>

#include <we/mgmt/layer.hpp>
#include <we/type/expression.fwd.hpp>
#include <we/type/literal.hpp>
#include <we/type/literal/read.hpp>
#include <we/type/module_call.fwd.hpp>
#include <we/type/net.fwd.hpp>
#include <we/type/value.hpp>
#include <we/type/value/read.hpp>
#include <we/util/token.hpp>

#include <fhg/revision.hpp>
#include <fhg/util/getenv.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/split.hpp>

#include <fhglog/fhglog.hpp>

#include <fstream>
#include <iostream>
#include <stdint.h>

#include <boost/program_options.hpp>

struct wfe_exec_context : public we::mgmt::context
{
  wfe_exec_context (we::loader::loader& module_loader)
    : loader (module_loader)
  {}

  virtual int handle_internally (we::mgmt::type::activity_t& act, net_t&)
  {
    act.inject_input ();

    while (act.can_fire())
    {
      we::mgmt::type::activity_t sub (act.extract ());
      sub.inject_input ();
      sub.execute (this);
      act.inject (sub);
    }

    act.collect_output ();

    return 0;
  }

  virtual int handle_internally (we::mgmt::type::activity_t& act, mod_t& mod)
  {
    module::call (loader, act, mod);

    return 0;
  }

  virtual int handle_internally (we::mgmt::type::activity_t& , expr_t&)
  {
    return 0;
  }

  virtual int handle_externally (we::mgmt::type::activity_t& act, net_t& n)
  {
    return handle_internally (act, n);
  }

  virtual int handle_externally (we::mgmt::type::activity_t& act, mod_t& mod)
  {
    return handle_internally (act, mod);
  }

  virtual int handle_externally (we::mgmt::type::activity_t& act, expr_t& e)
  {
    return handle_internally (act, e);
  }

private:
  we::loader::loader& loader;
};

int main (int argc, char **argv)
{
  FHGLOG_SETUP (argc, argv);

  namespace po = boost::program_options;

  po::options_description desc ("options");

  std::string path_to_act ("/dev/stdin");
  std::string mod_path;
  std::vector<std::string> mods_to_load;
  std::vector<std::string> input_spec;
  std::string output ("/dev/stdout");
  std::size_t num_worker (1);
  bool show_dots (false);

  desc.add_options()
    ("help,h", "this message")
    ("version,V", "print version information")
    ("verbose,v", "be verbose") // unused, we-exec interface compatibility
    ( "net"
    , po::value<std::string> (&path_to_act)->default_value (path_to_act)
    , "path to encoded activity or - for stdin"
    )
    ( "mod-path,L"
    , po::value<std::string> (&mod_path)->default_value
        (fhg::util::getenv("PC_LIBRARY_PATH", "."))
    , "where can modules be located"
    )
    ( "worker" // unused, we-exec interface compatibility
    , po::value<std::size_t> (&num_worker)->default_value(num_worker)
    , "number of workers"
    )
    ( "load"
    , po::value<std::vector<std::string> > (&mods_to_load)
    , "modules to load a priori"
    )
    ( "input,i"
    , po::value<std::vector<std::string> > (&input_spec)
    , "input token to the activity: <port>=<value>"
    )
    ( "output,o"
    , po::value<std::string> (&output)->default_value(output)
    , "output stream or - for stdout"
    )
    ( "show-dots,d" // unused, we-exec interface compatibility
    , po::value<bool> (&show_dots)->default_value(show_dots)
    , "show dots while waiting for progress"
    )
    ;

  po::positional_options_description p;
  p.add ("input", -1);

  po::variables_map vm;
  po::store ( po::command_line_parser(argc, argv)
            . options(desc).positional(p).run()
            , vm
            );
  po::notify (vm);

  if (vm.count ("help"))
  {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count ("version"))
  {
    std::cout << fhg::project_info();

    return EXIT_SUCCESS;
  }

  if (path_to_act == "-")
  {
    path_to_act = "/dev/stdin";
  }

  if (output == "-")
  {
    output = "/dev/stdout";
  }


  we::loader::loader loader;
  wfe_exec_context context (loader);

  BOOST_FOREACH (const std::string& module, mods_to_load)
  {
    loader.load (module);
  }

  {
    std::vector<std::string> search_path;
    fhg::log::split (mod_path, ":", std::back_inserter (search_path));
    BOOST_FOREACH (const std::string& path, search_path)
    {
      loader.append_search_path (path);
    }
  }

  we::mgmt::type::activity_t act;

  {
    std::ifstream ifs (path_to_act.c_str());
    if (!ifs)
    {
      std::cerr << "Could not open: " << path_to_act << std::endl;
      return 1;
    }

    we::util::codec::decode (ifs, act);
  }


  BOOST_FOREACH (const std::string& inp, input_spec)
  {
    const std::string port_name (inp.substr (0, inp.find ('=')));
    const std::string value (inp.substr (inp.find ('=') + 1));

    std::size_t k (0);
    std::string::const_iterator begin (value.begin());
    fhg::util::parse::position pos (k, begin, value.end());

    we::util::token::put (act, port_name, ::value::read (pos));
  }


  try
  {
    act.inject_input();
    act.execute (&context);
    act.collect_output ();

    std::ofstream stream (output.c_str());
    stream << act.to_string();
  }
  catch (std::exception const & ex)
  {
    std::cerr << "failed: " << ex.what() << std::endl;
    return 1;
  }

  return 0;
}
