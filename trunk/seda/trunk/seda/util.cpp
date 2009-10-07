#include "util.hpp"

namespace seda {
    /*
    unsigned long long getCurrentTimeMilliseconds() {
        struct timeval now;
        gettimeofday(&now, NULL);
        return now.tv_sec * 1000 + now.tv_usec;
    }
    */
    const unsigned long INFINITE_WAITTIME = 0xFFFFffff;
}
