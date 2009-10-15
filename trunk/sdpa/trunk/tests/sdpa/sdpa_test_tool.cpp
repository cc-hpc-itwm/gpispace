#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/LoggingConfigurator.hpp>

#include <sdpa/wf/Token.hpp>
#include <sdpa/wf/Parameter.hpp>
#include <sdpa/wf/Activity.hpp>

#include <gwdl/WFSerialization.h>

#include <iostream>

struct LogConfig {
    void operator()() {
        sdpa::logging::DefaultConfiguration()();
        std::clog << "private config stuff here." << std::endl;
    }
};

int main(int /* argc */, char ** /* argv */)
{
    using namespace sdpa;
    sdpa::logging::Configurator::configure(LogConfig());

    SDPA_DEFINE_LOGGER("tests.test-tool");

    SDPA_LOG_ERROR("test");

    sdpa::daemon::Job::ptr_t job(new sdpa::daemon::JobImpl("job-1234", "<job></job>"));
    std::clog << "logging enabled = " << SDPA_ENABLE_LOGGING << std::endl;
    std::clog << "log4cpp = " << SDPA_HAVE_LOG4CPP << std::endl;
    std::clog << "sdpa_logger = " << sdpa_logger.getName() << std::endl;
    std::clog << "id = " << job->id() << std::endl;
    std::clog << "desc = " << job->description() << std::endl;

    sdpa::wf::Activity activity("activity-1"
                              , sdpa::wf::Activity::Method("test.so", "loopStep")
                              , sdpa::wf::Activity::parameter_list_t());
    activity.add_parameter(sdpa::wf::Parameter
        ("i", sdpa::wf::Parameter::INPUT_EDGE, sdpa::wf::Token(42))
    );
    activity.add_parameter(sdpa::wf::Parameter
        ("o", sdpa::wf::Parameter::OUTPUT_EDGE, sdpa::wf::Token(""))
    );
    std::clog << "activity = " << activity << std::endl;

    std::clog << sdpa::wf::Activity::Method("test.so@loopStep") << std::endl;

    try
    {
      gwdl::deserializeWorkflow("foo");
    } catch (const std::runtime_error &re)
    {
      std::clog << "at least it throws now" << std::endl;  
    }

}
