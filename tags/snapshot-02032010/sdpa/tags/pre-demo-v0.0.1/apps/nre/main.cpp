#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <sdpa/sdpa-config.hpp>
#include <sdpa/logging.hpp>

#include <sdpa/daemon/nre/NRE.hpp>
#include <sdpa/util/Config.hpp>

namespace su = sdpa::util;

int main (int argc, char **argv) {
	sdpa::daemon::NRE::ptr_t ptrNRE = sdpa::daemon::NRE::create(sdpa::daemon::NRE, strAnswer );
	sdpa::daemon::NRE::start(ptrNRE, "127.0.0.1:5002", "127.0.0.1:5001");

	//sdpa::daemon::NRE::shutdown(ptrNRE);
}
