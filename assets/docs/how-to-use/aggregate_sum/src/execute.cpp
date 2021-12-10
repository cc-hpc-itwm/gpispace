#include <aggregate_sum/execute.hpp>
#include <aggregate_sum/Workflow.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <util-generic/executable_path.hpp>

#include <boost/filesystem/path.hpp>

#include <string>

namespace aggregate_sum
{
  namespace execution
  {
    ParametersDescription options()
    {
      namespace po = boost::program_options;

      ParametersDescription driver_opts ("Worker Topology");
      driver_opts.add_options()("topology", po::value<std::string>()->required());
      driver_opts.add (gspc::options::installation());
      driver_opts.add (gspc::options::drts());
      driver_opts.add (gspc::options::logging());
      driver_opts.add (gspc::options::scoped_rifd());

      return driver_opts;
    }
  }

  WorkflowResult execute (Parameters parameters, Workflow const& workflow)
  {
    auto const aggregate_sum_installation_path
      (fhg::util::executable_path().parent_path().parent_path());

    gspc::installation installation (parameters);
    gspc::scoped_rifds rifds(gspc::rifd::strategy {parameters},
                             gspc::rifd::hostnames {parameters},
                             gspc::rifd::port {parameters},
                             installation);

    gspc::set_application_search_path
      (parameters, aggregate_sum_installation_path / "lib");

    gspc::scoped_runtime_system drts (parameters,
                                      installation,
                                      parameters.at ("topology").as<std::string>(),
                                      rifds.entry_points());

    gspc::workflow const workflow_obj
      (aggregate_sum_installation_path / "pnet" / "aggregate_sum.pnet");

    return gspc::client {drts}.put_and_run (workflow_obj, workflow.inputs().map());
  }
}
