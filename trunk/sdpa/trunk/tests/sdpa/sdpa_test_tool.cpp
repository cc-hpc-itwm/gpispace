#include <sdpa/Job.hpp>
#include <sdpa/JobImpl.hpp>
#include <sdpa/logging.hpp>

#include <iostream>

int main(int argc, char **argv) {
    using namespace sdpa;
    SDPA_DEFINE_LOGGER("tests.test-tool");

    SDPA_LOG_ERROR("test");

    Job::ptr_t job(new JobImpl("job-1234", "<job></job>"));
    std::clog << "logging enabled = " << ENABLE_LOGGING << std::endl;
    std::clog << "log4cpp = " << HAVE_LOG4CPP << std::endl;
    std::clog << "sdpa_logger = " << sdpa_logger.getName() << std::endl;
    std::clog << "id = " << job->id() << std::endl;
    std::clog << "desc = " << job->description() << std::endl;
}
