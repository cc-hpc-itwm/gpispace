#include <sdpa/modules/Macros.hpp>
#include <iostream>
#include <string>
#include <cstdlib> // malloc, free
#include <fvm-pc/pc.hpp>
#include <remig/propagatorsOpt.h>
#include <stdexcept>

using namespace sdpa::modules;

/*
int po3d_ps_opt(
    float* const       u,        // complex u(nx, ny) frequency slice of wave field
    const int          nx,       //! number of grids in x-direction
    const int          ny,       //! number of grids in y-direction
    const float* const kx2,      // kx2(nx) square of wave-number array in x-direction
    const float* const ky2,      // ky2(ny) square of wave-number array in y-direction
    const float        k0,       //! reference wave-number
    const float        dz,       //! depth interval
    float* const       u1,       // temp space 4*roundUp4(nx) complex
    const int          iupd     //! up/down flag: -1=forward, +1=backword
 );
*/

static void calc(data_t &) throw (std::exception)
{
	po3d_ps_opt(
		NULL  // u
              , 0     // nx
              , 0     // ny
              , NULL  // kx2
              , NULL  // ky2
              , 0     // k0
              , 0     // dz
              , NULL  // temp space
              , -1    // iupd
        );
}

static void RunTest() throw (std::exception)
{
	data_t params;
	calc(params);
}

SDPA_MOD_INIT_START(remig)
{
  // run test cases
  RunTest();
}
SDPA_MOD_INIT_END(remig)
