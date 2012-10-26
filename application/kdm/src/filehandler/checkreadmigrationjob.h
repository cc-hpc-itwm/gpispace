/***************************************************************************
                          checkreadmigrationjob.h  -  description

    Read the variables for the migration job from the given
    configuration file and check for consitency of the data.
                             -------------------
    begin                : Mon Nov 27 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

       merten | 2010-01-28
                Support for ASCII config files has been removed
       merten | 2009-08-03
                Adapted to modified seisgrid3D class internal structure.
                The meaning of MigrationJob::SrfcGridX0/Y0 has been changed
                to model coordinate system.
      micheld | 2009-08-07
		parameter controlling rays per source in the raytracer changed from g_RAY_MIN to g_InitAngle
 ***************************************************************************/

#ifndef CHECKREADMIGRATIONJOB_H
#define CHECKREADMIGRATIONJOB_H

/**
  *@author Dirk Merten
  */

#include "include/defs.h"
#include "structures/migrationjob.h"
#include "structures/grid2d.h"
#include "structures/grid3d.h"
#include "filehandler/ttfilehandler.h"
#include "filehandler/tracefilehandler.h"
#include "utils/XMLReader_red.h"

#include <iostream>
#include <fstream>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

//

class CheckReadMigrationJob {

// public methods
public:
  CheckReadMigrationJob(){};

  /// Read configuration file 'ConfigFileName', check for consistent parameters and store parameters in 'Job'; return 0 on success, -1 on error
  int ReadConfigFileXML(char* ConfigFileName, MigrationJob& Job);

  /// Read configuration file 'ConfigFileName', check for consistent parameters and store parameters in 'Job'; return 0 on success, -1 on error
  int ReadTTTConfigFileXML(char* ConfigFileName, MigrationJob& Job);

  int WriteConfigFileXML(char* ConfigFileName, MigrationJob& Job);
  void Scale(MigrationJob& Job);
  ~CheckReadMigrationJob(){};

// public attributes
 public:

// public methods
 private:
  bool Check(MigrationJob& Job);
  int CheckExistence(const char* FileName);
  int CreateFile(const char* FileName);
  /// Read int variable Var with key VarName from CfgFileName; return true on success, false on error
  bool ReadXML(XMLReader& Reader, const char* VarName, int& Var, bool message = true);
  /// Read float variable Var with key VarName from CfgFileName; return true on success, false on error
  bool ReadXML(XMLReader& Reader, const char* VarName, float& Var, bool message = true);
  /// Read float variable Var with key VarName from CfgFileName; return true on success, false on error
  bool ReadXML(XMLReader& Reader, const char* VarName, char* Var, bool message = true);

// private attributes
 private:
  char* CfgFileName;

  //  
};
#endif

