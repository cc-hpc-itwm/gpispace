#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/LoggingConfigurator.hpp>

#include <sdpa/wf/Token.hpp>
#include <sdpa/wf/Parameter.hpp>
#include <sdpa/wf/Activity.hpp>

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

    sdpa::wf::Activity activity("activity-1", sdpa::wf::Activity::Method("test.so", "loopStep"));
    activity.add_input(sdpa::wf::Parameter
        (sdpa::wf::Token(0), "i", sdpa::wf::Parameter::INPUT_EDGE)
    );
    activity.add_output(sdpa::wf::Parameter
        (sdpa::wf::Token(), "i", sdpa::wf::Parameter::OUTPUT_EDGE)
    );
    std::clog << "activity = " << activity << std::endl;
}
