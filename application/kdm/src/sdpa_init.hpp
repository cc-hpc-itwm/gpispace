#ifndef SDPA_INIT_H
#define SDPA_INIT_H

#define VERSION "1.1"

// user defined includes
#include <MigVol.hpp>
#include <MigSubVol.hpp>
#include <TraceBunch.hpp>
#include <ttvmmemhandler.h>
#include "structures/migrationjob.h"
#include "structures/BlockVolume.h"
#include "utils/VM_ParallelEnvironment.h"
#include "filehandler/checkreadmigrationjob.h"
#include "filehandler/migrationfilehandler.h"

#include LOGGERINCLUDE 

// standard includes
#include <iostream>

extern LOGGER;

bool SDPA_init(int argc, char *argv[],MigrationJob &Job,ParallelEnvironment &PE,int NThreads=1);

#endif