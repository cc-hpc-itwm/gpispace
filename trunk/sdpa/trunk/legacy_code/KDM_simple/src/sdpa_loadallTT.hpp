#ifndef SDPA_LOADALLTT_H
#define SDPA_LOADALLTT_H

// user defined includes
#include <MigVol.hpp>
#include <MigSubVol.hpp>
#include <TraceBunch.hpp>
#include "ttvmmemhandler.h"
#include "structures/migrationjob.h"
#include "filehandler/checkreadmigrationjob.h"
#include "filehandler/migrationfilehandler.h"
#include "utils/VM_ParallelEnvironment.h"

#include LOGGERINCLUDE 

// standard includes
#include <iostream>

extern LOGGER;

bool SDPA_loadallTT(MigrationJob & _Job, ParallelEnvironment &_PE, int NThreads=1);

#endif