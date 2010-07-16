/***************************************************************************
                          checkreadtracingjob.h  -  description

    Read the variables for the tracing job from the given
    configuration file and check for consitency of the data.
                             -------------------
    begin                : Mon Nov 27 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

       merten | 2010-01-28
                Support for ASCII config files has been removed
       merten | 2009-11-05
                new variables for TTI added
       merten | 2009-11-05
                new variables for velicity file pre-processing included
      micheld | 2009-08-07
		parameter controlling rays per source changed from g_RAY_MIN to g_InitAngle
 ***************************************************************************/

#ifndef CHECKREADTRACINGJOB_H
#define CHECKREADTRACINGJOB_H

/**
  *@author Dirk Merten
  */

#include "include/defs.h"
#include "include/tracingtypes.h"
#include "structures/tracingjob.h"
#include "structures/optionalprm.h"
#include "utils/filename.h"
#include "utils/Acq_geometry.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>

//

class CheckReadTracingJob {

// public methods
 public:
    CheckReadTracingJob(){};
    ~CheckReadTracingJob(){};

    /// Read configuration file 'ConfigFileName', check for consistent parameters and store parameters in 'Job'; return 0 on success, -1 on error
    int ReadConfigFileXML(char* ConfigFileName, TracingJob& Job);

    /// Write configuration file 'ConfigFileName' containing the parameters of 'Job'; return 0 on success, -1 on error
    int WriteConfigFileXML(char* ConfigFileName, const TracingJob& Job);

    /// Write configuration file 'ConfigFileName' containing the parameters of 'Job'; return 0 on success, -1 on error
    int WriteTTConfigFileXML(char* ConfigFileName, const TracingJob& Job);

    /// Transform Job to model ccordinate system.
    void Scale( TracingJob& Job);

    int AddWorkingDir(char* CurrentDirectory, TracingJob& Job);

 // public attributes
 public:

// private methods
private:
    /// Try to open the configuration file CfgFileName; return 0 on success, -1 on error
    int CheckExistence();
    /// Read int variable Var with key VarName from CfgFileName; return true on success, false on error
    bool ReadXML(XMLReader& Reader, char* VarName, int& Var, bool message = true);
    /// Read float variable Var with key VarName from CfgFileName; return true on success, false on error
    bool ReadXML(XMLReader& Reader, char* VarName, float& Var, bool message = true);
    /// Read float variable Var with key VarName from CfgFileName; return true on success, false on error
    bool ReadXML(XMLReader& Reader, char* VarName, char* Var, bool message = true);
    /// Read "yes" or "no" string for a boolian variable Var with key VarName from CfgFileName; return true on success, false on error
    bool ReadXML(XMLReader& Reader, char* VarName, bool& Var, bool message = true);

// private attributes
 private:
    /// Temporal pointer to the name of the configuration file.
    char* CfgFileName;

    //    
};
#endif

