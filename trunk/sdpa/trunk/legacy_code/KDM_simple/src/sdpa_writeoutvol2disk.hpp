#ifndef SDPA_WRITEOC2DISK_H
#define SDPA_WRITEOC2DISK_H

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

bool SDPA_writeoc2disk(int _oid,MigrationJob &_Job,ParallelEnvironment &_PE);

#endif