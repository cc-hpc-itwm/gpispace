#include <sdpa/Job.hpp>
#include <sdpa/JobImpl.hpp>
#include <sdpa/LoggingConfigurator.hpp>

#include <sdpa/Token.hpp>
#include <sdpa/Parameter.hpp>
#include <sdpa/Activity.hpp>

#include <iostream>

struct LogConfig {
    void operator()() {
        sdpa::logging::DefaultConfiguration()();
        std::clog << "private config stuff here." << std::endl;
    }
};

int main(int argc, char **argv) {
    using namespace sdpa;
    sdpa::logging::Configurator::configure(LogConfig());

    SDPA_DEFINE_LOGGER("tests.test-tool");

    SDPA_LOG_ERROR("test");

    Job::ptr_t job(new JobImpl("job-1234", "<job></job>"));
    std::clog << "logging enabled = " << SDPA_ENABLE_LOGGING << std::endl;
    std::clog << "log4cpp = " << SDPA_HAVE_LOG4CPP << std::endl;
    std::clog << "sdpa_logger = " << sdpa_logger.getName() << std::endl;
    std::clog << "id = " << job->id() << std::endl;
    std::clog << "desc = " << job->description() << std::endl;

    sdpa::Activity activity("activity-1", sdpa::Activity::Method("test.so", "loopStep"));
    activity.add_input(sdpa::Parameter
        (sdpa::Token(0), "i", sdpa::Parameter::INPUT_EDGE)
    );
    activity.add_output(sdpa::Parameter
        (sdpa::Token(), "i", sdpa::Parameter::OUTPUT_EDGE)
    );
    std::clog << "activity = " << activity << std::endl;
}
