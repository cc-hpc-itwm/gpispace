#include <iostream>
#include <sstream>

#include <stdint.h>
#include <we/we.hpp>
#include <we/mgmt/layer.hpp>

#include <kdm/simple_generator.hpp>

#include <boost/program_options.hpp>
#include "test_layer.hpp"

using namespace we::mgmt;
using namespace test;
namespace po = boost::program_options;

typedef std::string id_type;
//typedef uint64_t id_type;

typedef we::mgmt::layer<id_type, we::activity_t> layer_t;
typedef sdpa_daemon<layer_t> daemon_type;
typedef layer_t::internal_id_type layer_id_type;

std::vector<id_type> jobs;

// observe workflow engine
static
void observe_submitted (const layer_t *, layer_id_type const &)
{
}
static
void observe_finished (const layer_t *l, layer_id_type const & id, std::string const &s)
{
  we::activity_t act (layer_t::policy::codec::decode (s));
  if (act.transition().name() == "trans_net")
  {
    std::cerr << "job finished: id := " << id << std::endl;
    l->print_statistics(std::cerr);
    jobs.pop_back();
  }
}
static
void observe_failed (const layer_t *, layer_id_type const & id, std::string const &)
{
  std::cerr << "activity failed: id := " << id << std::endl;
}
static
void observe_cancelled (const layer_t *, layer_id_type const & id, std::string const &)
{
  std::cerr << "activity cancelled: id := " << id << std::endl;
}
static
void observe_executing (const layer_t *, layer_id_type const &)
{
}

int main (int argc, char **argv)
{
  po::options_description desc("options");

  std::string cfg_file;
  std::string mod_path;
  std::vector<std::string> mods_to_load;

  const std::size_t num_jobs = 1;
  const std::size_t num_worker = 1;

  desc.add_options()
    ("help", "this message")
    ("cfg", po::value<std::string>(&cfg_file)->default_value("/scratch/KDM/KDM.conf"), "config file")
    ("mod-path", po::value<std::string>(&mod_path)->default_value("/scratch/KDM/"), "modules")
    ("load", po::value<std::vector<std::string> >(&mods_to_load), "modules to load a priori")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
    {
      std::cout << desc << std::endl;
      return EXIT_SUCCESS;
    }

  // instantiate daemon and layer
  daemon_type daemon(num_worker);

  for ( std::vector<std::string>::const_iterator m (mods_to_load.begin())
      ; m != mods_to_load.end()
      ; ++m
      )
  {
    daemon.loader().load (*m, *m);
  }

  daemon.loader().append_search_path (mod_path);
  daemon_type::layer_type & mgmt_layer = daemon.layer();

  mgmt_layer.sig_submitted.connect ( &observe_submitted );
  mgmt_layer.sig_finished.connect  ( &observe_finished );
  mgmt_layer.sig_failed.connect    ( &observe_failed );
  mgmt_layer.sig_cancelled.connect ( &observe_cancelled );
  mgmt_layer.sig_executing.connect ( &observe_executing );

  for (std::size_t i (0); i < num_jobs; ++i)
  {
    we::transition_t simple_trans (kdm::kdm<we::activity_t>::generate());
    we::activity_t act ( simple_trans );
    act.input().push_back
      ( we::input_t::value_type
        ( we::token_t ( "config_file"
                      , literal::STRING
                      , cfg_file
                      )
        , simple_trans.input_port_by_name ("config_file")
      )
    );

    daemon_type::id_type id = daemon.gen_id();
    jobs.push_back(id);
    mgmt_layer.submit(id, layer_t::policy::codec::encode (act));
  }

  for (std::vector<id_type>::const_iterator id (jobs.begin()); id != jobs.end(); ++id)
  {
    try
    {
      // mgmt_layer.suspend(*id);
      // mgmt_layer.resume(*id);
      // mgmt_layer.suspend(*id);
      // mgmt_layer.resume(*id);
    }
    catch (const std::runtime_error & ex)
    {
      std::cerr << "mgmt layer removed activity " << *id << ": " << ex.what() << std::endl;
    }
  }

#if 0
  size_t max_wait (5);

  while (jobs.size() > 0 && (--max_wait > 0))
  {
    std::cerr << "." << std::flush;
    sleep (1);
  }
#else
  while (jobs.size() > 0)
  {
    std::cerr << "." << std::flush;
    sleep (1);
  }
#endif

  std::cerr << "Everything done." << std::endl;

  return ((jobs.size() == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
}
