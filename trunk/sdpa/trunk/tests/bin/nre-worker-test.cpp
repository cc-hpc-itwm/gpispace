/*
 * =====================================================================================
 *
 *       Filename:  test_worker.cpp
 *
 *    Description:  tests the nre worker component
 *
 *        Version:  1.0
 *        Created:  11/12/2009 12:23:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <fhglog/fhglog.hpp>
#include <fhglog/util.hpp>
#include <sdpa/daemon/nre/NreWorkerClient.hpp>
#include <sdpa/wf/Activity.hpp>

int main(int ac, char **av)
{
  fhg::log::Configurator::configure();

  std::string worker_location("127.0.0.1:8000");
  std::string function_call("init@init");
  if (ac > 1)
  {
    worker_location = av[1];
  }
  if (ac > 2)
  {
    function_call = av[2];
  }

  sdpa::wf::parameters_t params;
  for (int i = 3; i < ac; ++i)
  {
    // param = datatype:value
    // param = value
    std::pair<std::string, std::string> kv = fhg::log::split_string(av[i], "=");

    std::string p_name = kv.first;
    std::string value  = kv.second;
    std::string datatype = "unknown";

    kv = fhg::log::split_string(kv.second, ":");
    if (kv.second.empty()) value = kv.first;
    else if (kv.first.empty()) value = kv.second;
    else
    {
      datatype = kv.first;
      value = kv.second;
    }

    // TODO: parse edge type
    std::cout << "adding parameter (type " << datatype << ") " << p_name << "=" << value << "" << std::endl;

    sdpa::wf::Parameter p(p_name, sdpa::wf::Parameter::INPUT_EDGE, sdpa::wf::Token(value));
    p.token().properties().put("datatype", datatype);
    params[p_name] = p;
  }

  sdpa::nre::worker::NreWorkerClient client(worker_location);
  client.start();

  client.ping();

  // try to execute an activity
  sdpa::wf::Activity req("activity-1", sdpa::wf::Method(function_call), params);
  req.properties().put("keep_going", true);

  std::cout << "sending ";
  req.writeTo(std::cout, false);
  std::cout << std::endl;

  sdpa::wf::Activity res(client.execute(req));
  std::cout << "got ";
  res.writeTo(std::cout, false);
  std::cout << std::endl;

  std::cout << std::endl;
  std::cout << "========= " << ((res.state() == sdpa::wf::Activity::ACTIVITY_FINISHED) ? ("finished") : ("failed")) << " ========" << std::endl;
  std::cout << res.reason() << std::endl;
  for (sdpa::wf::parameters_t::const_iterator out(res.parameters().begin()); out != res.parameters().end(); ++out)
  {
    std::cout << out->first << "=" << out->second.token().properties().get("datatype") << ":" << "\"" << out->second.token().data() << "\"" << std::endl;
  }

  client.stop();
  return 0;
}

