#include <iostream>
#include <fstream>
#include <sstream>

#include <stdint.h>
#include <fhglog/fhglog.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/getenv.hpp>

#include <we/we.hpp>
#include <we/mgmt/layer.hpp>
#include <we/type/token.hpp>
#include <we/type/literal.hpp>
#include <we/type/literal/read.hpp>

#include <fhg/util/parse/position.hpp>

#include <fhg/revision.hpp>

#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include "test_layer.hpp"

using namespace we::mgmt;
using namespace test;
namespace po = boost::program_options;

typedef std::string id_type;

typedef we::mgmt::layer layer_t;
typedef sdpa_daemon<layer_t> daemon_type;
typedef layer_t::internal_id_type layer_id_type;

static std::vector<id_type> jobs;
static std::set<layer_id_type> layer_jobs;
static boost::recursive_mutex mutex;
typedef boost::unique_lock<boost::recursive_mutex> lock_t;
static bool verbose (false);

// observe workflow engine
static
void observe_submitted (const layer_t *l, layer_id_type const & id)
{
  {
    lock_t lock(mutex);

    std::cerr << "submitted: " << id << std::endl;
    layer_jobs.insert (id);
  }

  if (verbose)
    l->print_statistics(std::cerr);
}

static
void observe_finished (const layer_t *l, layer_id_type const & id, std::string const &s)
{
  {
    lock_t lock(mutex);

    if (layer_jobs.find (id) != layer_jobs.end())
    {
      layer_jobs.erase (id);
      we::activity_t act (we::util::codec::decode (s));
      std::cerr << "job finished: " << act.transition().name() << "-" << id << std::endl;
    }
  }

  if (verbose)
    l->print_statistics(std::cerr);
}
static
void observe_failed (const layer_t *l, layer_id_type const & id, std::string const &s)
{
  {
    lock_t lock(mutex);

    if (layer_jobs.find (id) != layer_jobs.end())
    {
      layer_jobs.erase (id);
      we::activity_t act (we::util::codec::decode (s));
      std::cerr << "job failed: " << act.transition().name() << "-" << id << std::endl;
    }
  }

  if (verbose)
    l->print_statistics(std::cerr);
}
static
void observe_cancelled (const layer_t *l, layer_id_type const & id, std::string const &s)
{
  {
    lock_t lock(mutex);

    if (layer_jobs.find (id) != layer_jobs.end())
    {
      layer_jobs.erase (id);
      we::activity_t act (we::util::codec::decode (s));
      std::cerr << "job cancelled: " << act.transition().name() << "-" << id << std::endl;
    }
  }

  if (verbose)
    l->print_statistics(std::cerr);
}

static
void observe_executing (const layer_t *l, layer_id_type const & id)
{
  std::cerr << "activity executing: id := " << id << std::endl;
  if (verbose)
    l->print_statistics(std::cerr);
}

int main (int argc, char **argv)
{
  fhg::log::Configurator::configure();

  po::options_description desc("options");

  std::string path_to_act;
  std::string mod_path;
  std::vector<std::string> mods_to_load;
  std::vector<std::string> input_spec;
  std::size_t num_worker (8);
  std::string output ("-");
  bool show_dots (false);

  desc.add_options()
    ("help,h", "this message")
    ("version,V", "print version information")
    ("verbose,v", "be verbose")
    ("net", po::value<std::string>(&path_to_act)->default_value("-"), "path to encoded activity or - for stdin")
    ( "mod-path,L"
    , po::value<std::string>(&mod_path)->default_value
        (fhg::util::getenv("PC_LIBRARY_PATH", "."))
    , "where can modules be located"
    )
    ("worker", po::value<std::size_t>(&num_worker)->default_value(num_worker), "number of workers")
    ("load", po::value<std::vector<std::string> >(&mods_to_load), "modules to load a priori")
    ("input,i", po::value<std::vector<std::string> >(&input_spec), "input token to the activity: port=<value>")
    ("output,o", po::value<std::string>(&output)->default_value(output), "output stream (ignored)")
    ("show-dots,d", po::value<bool>(&show_dots)->default_value(show_dots), "show dots while waiting for progress")
    ;

  po::positional_options_description p;
  p.add("input", -1);

  po::variables_map vm;
  po::store( po::command_line_parser(argc, argv)
           . options(desc).positional(p).run()
           , vm
           );
  po::notify (vm);

  if (vm.count("help"))
    {
      std::cout << desc << std::endl;
      return EXIT_SUCCESS;
    }

  if (vm.count("version"))
  {
    std::cout << fhg::project_info();

    return EXIT_SUCCESS;
  }

  // instantiate daemon and layer
  daemon_type daemon(num_worker);

  for ( std::vector<std::string>::const_iterator m (mods_to_load.begin())
      ; m != mods_to_load.end()
      ; ++m
      )
  {
    daemon.loader().load (*m);
  }

  {
    std::vector<std::string> search_path;
    fhg::log::split (mod_path, ":", std::back_inserter (search_path));
    BOOST_FOREACH (std::string const &p, search_path)
    {
      daemon.loader().append_search_path (p);
    }
  }
  daemon_type::layer_type & mgmt_layer = daemon.layer();

  mgmt_layer.sig_submitted.connect ( &observe_submitted );
  mgmt_layer.sig_finished.connect  ( &observe_finished );
  mgmt_layer.sig_failed.connect    ( &observe_failed );
  mgmt_layer.sig_cancelled.connect ( &observe_cancelled );

  if (vm.count ("verbose"))
  {
    verbose = true;
    mgmt_layer.sig_executing.connect ( &observe_executing );
  }

  we::activity_t act;

  if (path_to_act != "-")
  {
    std::ifstream ifs (path_to_act.c_str());
    if (! ifs)
    {
      std::cerr << "Could not open: " << path_to_act << std::endl;
      return 1;
    }
    act = we::util::codec::decode (ifs);
  }
  else
  {
    std::cerr << "Reading from stdin..." << std::endl;
    act = we::util::codec::decode (std::cin);
  }

  for ( std::vector<std::string>::const_iterator inp (input_spec.begin())
      ; inp != input_spec.end()
      ; ++inp
      )
  {
    const std::string port_name
      ( inp->substr (0, inp->find('=') ));
    const std::string value
      ( inp->substr (inp->find('=')+1) );

    literal::type tokval;
    std::size_t k (0);
    std::string::const_iterator begin (value.begin());

    fhg::util::parse::position pos (k, begin, value.end());
    literal::read (tokval, pos);

    act.add_input (
                   we::input_t::value_type
                   ( token::type ( port_name
                                 , boost::apply_visitor (literal::visitor::type_name(), tokval)
                                 , tokval
                                 )
                   , act.transition().input_port_by_name (port_name)
                   )
                  );
  }

  daemon_type::id_type id = daemon.gen_id();
  jobs.push_back(id);
  mgmt_layer.submit(id, act.to_string());

#if 0
  size_t max_wait (5);

  while (jobs.size() > 0 && (--max_wait > 0))
    {
      if (show_dots) { std::cerr << "." << std::flush; }
      sleep (1);
    }
#else
  while (layer_jobs.size() > 0)
    {
      if (show_dots) { std::cerr << "." << std::flush; }
      sleep (1);
    }
#endif

#ifdef STATISTICS_CONDITION
  statistics::dump_maps();
#endif

  std::cerr << "Everything done." << std::endl;

  return ((jobs.size() == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
}
