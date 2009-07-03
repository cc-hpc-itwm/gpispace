#include <iostream>

#include <sdpa/sdpa-config.hpp>
#include <sdpa/client/Client.hpp>

int main (int argc, char **argv) {
  std::cerr << "SDPA - Seismic Data Processing Architecture" << std::endl;
  std::cerr << "===========================================" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Version: " << SDPA_VERSION << std::endl;
  std::cerr << "Copyright 2009 - Fraunhofer ITWM" << std::endl;
  std::cerr << "Contact: alexander.petry@itwm.fraunhofer.de" << std::endl;
  std::cerr << std::endl;

  sdpa::client::Client::ptr_t api(sdpa::client::Client::create());

  api->start();

  sdpa::job_id_t jid = api->submitJob("<xml></xml>");
  api->queryJob(jid);
  api->cancelJob(jid);
  api->retrieveResults(jid);
  api->deleteJob(jid);

  api->shutdown();
}
