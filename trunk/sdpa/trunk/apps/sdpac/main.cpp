#include <iostream>

#include <sdpa/sdpa-config.hpp>
#include <sdpa/client/ClientApi.hpp>

int main (int argc, char **argv) {
  sdpa::client::ClientApi::ptr_t api(sdpa::client::ClientApi::create("empty config"));

  if (argc > 1 && (std::string(argv[1]) == "-v"))
  {
    std::cerr << "           "
              << "SDPA - Seismic Data Processing Architecture" << std::endl;
    std::cerr << "           "
              << "===========================================" << std::endl;
    std::cerr << "                            "
              << "v" << api->version()
              << std::endl
              << "                 "
              << " " << api->copyright()
              << std::endl;
    std::cerr << "       "
              << api->contact()
              << std::endl;
    std::cerr << std::endl;
  }


  sdpa::job_id_t jid = api->submitJob("<xml></xml>");
  api->queryJob(jid);
  api->cancelJob(jid);
  api->retrieveResults(jid);
  api->deleteJob(jid);
}
