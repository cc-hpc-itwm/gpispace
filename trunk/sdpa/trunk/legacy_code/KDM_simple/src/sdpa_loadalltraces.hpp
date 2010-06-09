#ifndef SDPA_LOADALLTRACES_H
#define SDPA_LOADALLTRACES_H

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

bool SDPA_loadalltraces(int _oid, MigrationJob & _Job, ParallelEnvironment &_PE);

bool SDPA_detbunchloc(const int &_oid, const int &_pid, const int &_bid, const MigrationJob &_Job, 
                      ParallelEnvironment &PE, int &_relgloboff, int &_VMStoreRank);

#endif