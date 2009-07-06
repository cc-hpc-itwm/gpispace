#include <iostream>

#include <sdpa/sdpa-config.hpp>
#include <sdpa/client/ClientApi.hpp>

int main (int argc, char **argv) {
  std::cerr << "           "
            << "SDPA - Seismic Data Processing Architecture" << std::endl;
  std::cerr << "           "
            << "===========================================" << std::endl;
  std::cerr << "                            "
            << "v" << SDPA_VERSION
            << std::endl
            << "                 "
            << " " << SDPA_COPYRIGHT
            << std::endl;
  std::cerr << "       "
            << SDPA_CONTACT
            << std::endl;
  std::cerr << std::endl;

  sdpa::client::ClientApi::ptr_t api(sdpa::client::ClientApi::create("empty config"));

  sdpa::job_id_t jid = api->submitJob("<xml></xml>");
  api->queryJob(jid);
  api->cancelJob(jid);
  api->retrieveResults(jid);
  api->deleteJob(jid);
}
