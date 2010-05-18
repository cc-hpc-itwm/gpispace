#include <iostream>
#include <sstream>

#include <stdint.h>
#include <we/we.hpp>
#include <we/mgmt/layer.hpp>

#include <kdm/simple_generator.hpp>

#include "test_layer.hpp"

using namespace we::mgmt;
using namespace test;

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
  const std::string cfg_file = std::string ((argc > 1) ? argv[1] : "/scratch/KDM/KDM.conf");
  const std::string mod_path = std::string ((argc > 2) ? argv[2] : "/scratch/KDM/");
  const std::size_t num_jobs = ((argc > 3) ? (size_t)atoi(argv[3]) : 1);
  const std::size_t num_worker = ((argc > 4) ? (size_t)atoi(argv[4]) : 1);

  // instantiate daemon and layer
  daemon_type daemon(num_worker);
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

  return ( (num_jobs == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
}
