#include <sdpa/Job.hpp>
#include <sdpa/JobImpl.hpp>

#include <iostream>

int main(int argc, char **argv) {
    using namespace sdpa;

    Job::ptr_t job(new JobImpl("job-1234", "<job></job>"));
    std::clog << "id = " << job->id() << std::endl;
    std::clog << "desc = " << job->description() << std::endl;
}
