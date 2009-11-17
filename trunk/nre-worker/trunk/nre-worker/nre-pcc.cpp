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

#include <boost/program_options.hpp>

typedef std::vector<std::string>::const_iterator param_iterator;
bool verbose(false);

void parse_parameters(param_iterator begin, param_iterator end, sdpa::wf::parameters_t & params, sdpa::wf::Parameter::EdgeType edge_type)
{
  for (param_iterator p_it(begin); p_it != end; ++p_it)
  {
    // param = datatype:value
    // param = value
    std::pair<std::string, std::string> kv = fhg::log::split_string(*p_it, "=");

    std::string p_name = kv.first;
    const std::string value  = kv.second;
    std::string datatype = "unknown";

    kv = fhg::log::split_string(p_name, ":");

    if (kv.second.empty()) p_name = kv.first;
    else if (kv.first.empty()) p_name = kv.second;
    else
    {
      datatype = kv.first;
      p_name = kv.second;
    }

    sdpa::wf::Parameter p(p_name, edge_type, sdpa::wf::Token(value));
    params[p_name] = p;
    p.token().properties().put("datatype", datatype);

    if (verbose)
    {
      std::cout << "added ";
      switch (edge_type)
      {
        case sdpa::wf::Parameter::INPUT_EDGE:
          std::cout << "input"; break;
        case sdpa::wf::Parameter::READ_EDGE:
          std::cout << "read"; break;
        case sdpa::wf::Parameter::OUTPUT_EDGE:
          std::cout << "output"; break;
        case sdpa::wf::Parameter::WRITE_EDGE:
          std::cout << "write"; break;
        case sdpa::wf::Parameter::EXCHANGE_EDGE:
          std::cout << "exchange"; break;
        case sdpa::wf::Parameter::UPDATE_EDGE:
          std::cout << "update"; break;
          break;
      } 
      std::cout << " parameter: " << p << std::endl;
    }
  }
}

int main(int ac, char **av)
{
  namespace po = boost::program_options;

  po::options_description opts("Available Options");
  // fill in defaults
  opts.add_options()
    ("help,h", "show this help text")
    ("worker", po::value<std::string>()->default_value("127.0.0.1:8000"), "location of the nre-pcd")

    ("iparam,i", po::value<std::vector<std::string> >(), "input  parameter to the activity")
    ("oparam,o", po::value<std::vector<std::string> >(), "output parameter to the activity")
    ("wparam,w", po::value<std::vector<std::string> >(), "write  parameter to the activity")
    ("rparam,r", po::value<std::vector<std::string> >(), "read   parameter to the activity")

    ("wparam" , po::value<std::vector<std::string> >(), "write parameter to the activity")
    ("keep-going,k", "keep going")
    ("verbose,v", "verbose output")
    ("function,f", po::value<std::string>(), "the function to be called")
  ;

  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(ac, av).options(opts)
                                             .run()
            , vm);
  }
  catch (const std::exception &ex)
  {
    std::cerr << "could not parse command line: " << ex.what() << std::endl;
    std::cerr << opts << std::endl;
    return 2;
  }

  if (vm.count("help"))
  {
    std::cerr << "usage: nre-pcc [options] out-params...." << std::endl;
    std::cerr << opts << std::endl;
    return 0;
  }

  if (vm.count("function") == 0)
  {
    std::cerr << "E: function to be called is missing!" << std::endl;
    return 2;
  }

  fhg::log::Configurator::configure();

  verbose = vm.count("verbose") > 0;
  std::string function_call(vm["function"].as<std::string>());

  std::string worker_location(vm["worker"].as<std::string>());
  sdpa::nre::worker::NreWorkerClient client(worker_location);

  try
  {
    client.start();
  }
  catch (const std::exception &ex)
  {
    std::cerr << "E: could not connect to nre-pcd: " << ex.what() << std::endl;
    return 3;
  }

  sdpa::wf::parameters_t params;
  {
    if (vm.count("iparam"))
    {
      const std::vector<std::string> &i_params = vm["iparam"].as<std::vector<std::string> >();
      parse_parameters(i_params.begin(), i_params.end(), params, sdpa::wf::Parameter::INPUT_EDGE);
    }

    if (vm.count("oparam"))
    {
      const std::vector<std::string> &o_params = vm["oparam"].as<std::vector<std::string> >();
      parse_parameters(o_params.begin(), o_params.end(), params, sdpa::wf::Parameter::OUTPUT_EDGE);
    }

    if (vm.count("rparam"))
    {
      const std::vector<std::string> &r_params = vm["rparam"].as<std::vector<std::string> >();
      parse_parameters(r_params.begin(), r_params.end(), params, sdpa::wf::Parameter::READ_EDGE);
    }

    if (vm.count("wparam"))
    {
      const std::vector<std::string> &w_params = vm["wparam"].as<std::vector<std::string> >();
      parse_parameters(w_params.begin(), w_params.end(), params, sdpa::wf::Parameter::WRITE_EDGE);
    }
  }

  // try to execute an activity
  sdpa::wf::Activity req("activity-1", sdpa::wf::Method(function_call), params);
  if (vm.count("keep-going")) req.properties().put("keep_going", true);

  std::cout << "executing ";
  req.writeTo(std::cout, false);
  std::cout << "..." << std::endl;

  sdpa::wf::Activity res;
  try
  {
    res = client.execute(req);
  }
  catch (const std::exception &ex)
  {
    std::cerr << "E: activity execution failed: " << ex.what() << std::endl;
    return 3;
  }

  if (verbose)
  {
    std::cout << "got ";
    res.writeTo(std::cout, false);
    std::cout << std::endl;
  }

  std::cout << std::endl;
  std::cout << "========= " << ((res.state() == sdpa::wf::Activity::ACTIVITY_FINISHED) ? ("finished") : ("failed")) << " ========" << std::endl;
  std::cout << res.reason() << std::endl;
  for (sdpa::wf::parameters_t::const_iterator out(res.parameters().begin()); out != res.parameters().end(); ++out)
  {
    const sdpa::wf::Parameter &p(out->second);
    const std::string &dtype = p.token().properties().get("datatype");

    std::cout << "\t";
    if (p.edge_type() == sdpa::wf::Parameter::INPUT_EDGE)
    {
      std::cout << "-i ";
    }
    else if (p.edge_type() == sdpa::wf::Parameter::OUTPUT_EDGE)
    {
      std::cout << "-o ";
    }
    else
    {
      std::cout << "-p ";
    }
    std::cout << dtype << ":" << out->first << "=" << "\"" << p.token().data() << "\"" << std::endl;

    if (out->first != p.name())
    {
      std::cerr << "\t\t*** inconsistency detected, make sure *all* parameters (input/output) are defined before execution!" << std::endl;
    }
  }

  client.stop();
  return 0;
}
