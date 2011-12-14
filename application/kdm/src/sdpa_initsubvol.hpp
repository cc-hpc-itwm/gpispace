#ifndef SDPA_INITSUBVOL_H
#define SDPA_INITSUBVOL_H

// user defined includes
#include <MigVol.hpp>
#include <MigSubVol.hpp>
#include <TraceBunch.hpp>
#include "structures/migrationjob.h"
#include "filehandler/checkreadmigrationjob.h"
#include "filehandler/migrationfilehandler.h"
#include "utils/VM_ParallelEnvironment.h"

#include LOGGERINCLUDE 

// standard includes
#include <iostream>

extern LOGGER;

bool SDPA_initsubvol(MigrationJob &_Job,ParallelEnvironment &PE);

#endif