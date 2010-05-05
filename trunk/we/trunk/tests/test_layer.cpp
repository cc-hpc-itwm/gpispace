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

// observe workflow engine
static
void observe_submitted (layer_id_type const & id, std::string const & str)
{
  std::cerr << "activity submitted: id := " << id << " data := " << str << std::endl;
}
static
void observe_finished (layer_id_type const & id, std::string const & str)
{
  std::cerr << "activity finished: id := " << id << " data := " << str << std::endl;
}
static
void observe_failed (layer_id_type const & id, std::string const & str)
{
  std::cerr << "activity failed: id := " << id << " data := " << str << std::endl;
}
static
void observe_cancelled (layer_id_type const & id, std::string const & str)
{
  std::cerr << "activity cancelled: id := " << id << " data := " << str << std::endl;
}
static
void observe_executing (layer_id_type const & id, std::string const & str)
{
  std::cerr << "activity executing: id := " << id << " data := " << str << std::endl;
}

int main (int argc, char **argv)
{
  // instantiate daemon and layer
  daemon_type daemon;
  daemon_type::layer_type & mgmt_layer = daemon.layer();
  mgmt_layer.sig_submitted.connect ( &observe_submitted );
  mgmt_layer.sig_finished.connect  ( &observe_finished );
  mgmt_layer.sig_failed.connect    ( &observe_failed );
  mgmt_layer.sig_cancelled.connect ( &observe_cancelled );
  mgmt_layer.sig_executing.connect ( &observe_executing );

  std::vector<id_type> ids;

  for (std::size_t i (0); i < 1; ++i)
  {
    we::transition_t simple_trans (kdm::kdm<we::activity_t>::generate());
    we::activity_t act ( simple_trans );
    act.input().push_back
      ( we::input_t::value_type
        ( we::token_t ( "config_file"
                      , literal::STRING
                      , std::string ((argc > 1) ? argv[1] : "/scratch/KDM.conf")
                      )
        , simple_trans.input_port_by_name ("config_file")
      )
    );

    daemon_type::id_type id = daemon.gen_id();
    ids.push_back(id);
    mgmt_layer.submit(id, layer_t::policy::codec::encode (act));
  }

  for (std::vector<id_type>::const_iterator id (ids.begin()); id != ids.end(); ++id)
  {
    mgmt_layer.suspend(*id);
    sleep(3);
    mgmt_layer.resume(*id);
    //	mgmt_layer.failed(*id, "");
    //	mgmt_layer.finished(*id, "");
    mgmt_layer.cancel(*id, "");
    sleep(1);
    mgmt_layer.suspend(*id);
  }

  sleep(1); // not nice, but we cannot wait for a network to finish right now

  return EXIT_SUCCESS;
}
