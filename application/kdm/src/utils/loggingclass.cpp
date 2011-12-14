/***************************************************************************
                          loggingclass.cpp -  description

    Read the variables for the migration job from the given
    configuration file and check for consitency of the data.
                             -------------------
    begin                : Mon Nov 27 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

#include "loggingclass.h"
 int LoggingClass::mode = 1;
 bool LoggingClass::tofile = false;
 char LoggingClass::LogFileName[199];
