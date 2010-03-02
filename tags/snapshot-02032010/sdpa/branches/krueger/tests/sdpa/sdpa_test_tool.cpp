#include <sdpa/Job.hpp>
#include <sdpa/JobImpl.hpp>
#include <sdpa/LoggingConfigurator.hpp>

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
    std::clog << "logging enabled = " << ENABLE_LOGGING << std::endl;
    std::clog << "log4cpp = " << HAVE_LOG4CPP << std::endl;
    std::clog << "sdpa_logger = " << sdpa_logger.getName() << std::endl;
    std::clog << "id = " << job->id() << std::endl;
    std::clog << "desc = " << job->description() << std::endl;
}
