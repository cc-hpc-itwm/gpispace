/************************************************************************
                        loggingclass.h  -  description
 
*//**   Print logging information to stdout and/or log file.
        All print functions accept 'printf-style' format strings. 
*//* 
                            -------------------
  begin                : Mon Nov 27 2006
 
  copyright            : (C) 2006 by Dirk Merten
  email                : merten@itwm.fhg.de
 ***************************************************************************/

//#define USE_PV4D_LOGGER

#ifdef USE_PV4D_LOGGER
#ifdef __ALTIVEC__
#define VM3
#include "Pv4dLogger.h"
#else
#include "Pv4dVMLogger.h"
#endif
#endif

#ifndef LOGGINGCLASS_H
#define LOGGINGCLASS_H

/*
  *@author Dirk Merten
  *@date 27.11. 2006
  */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define MAX_MESSAGE_LENGTH 200

class LoggingClass {

// public methods
public: 
    LoggingClass(){};
    ~LoggingClass(){};
    /// Initialize the log file 
    int SetFile(const char* _LogFileName)
	{
	    if ( (_LogFileName != NULL) && (strlen(_LogFileName) > 0))
	    {
		sprintf(&LogFileName[0], _LogFileName);
		FILE* LogFile = fopen(LogFileName, "a");
		if (LogFile == NULL)
		{
		    printf("WARNING     : Could not open Log File\n");
		    printf("WARNING     : %s.\n", LogFileName);
		    return -1;
		}
		fclose(LogFile);
		tofile = true;
	    }
	    else
		tofile = false;
	    return 0;
	};
    /// Create the log file 
    int CreateFile(const char* _LogFileName)
	{
	    if ( (_LogFileName != NULL) && (strlen(_LogFileName) > 0))
	    {
		sprintf(&LogFileName[0], _LogFileName);
		FILE* LogFile = fopen(LogFileName, "w");
		if (LogFile == NULL)
		{
		    printf("WARNING     : Could not open Log File\n");
		    printf("WARNING     : %s.\n", LogFileName);
		    return -1;
		}
		fprintf(LogFile, "      \n");
		fclose(LogFile);
		tofile = true;
	    }
	    else
		tofile = false;
	    return 0;
	};

    /// Initialize the log file depending on the rank of the caller 
    int SetFile(const char* _LogFileName, const int& _rank)
	{
	    if ( (_LogFileName != NULL) && (strlen(_LogFileName) > 0))
	    {
		sprintf(&LogFileName[0], "%s_%i", _LogFileName, _rank);
		FILE* LogFile = fopen(LogFileName, "a");
		if (LogFile == NULL)
		{
		    printf("WARNING     : Could not open Log File\n");
		    printf("WARNING     : %s.\n", LogFileName);
		    return -1;
		}
		fclose(LogFile);
		tofile = true;
	    }
	    else 
		tofile = false;
	    return 0;
	};

    /// Create the log file depending on the rank of the caller
    int CreateFile(const char* _LogFileName, const int& _rank)
	{
	    if ( (_LogFileName != NULL) && (strlen(_LogFileName) > 0))
	    {
		sprintf(&LogFileName[0], "%s_%i", _LogFileName, _rank);
		FILE* LogFile = fopen(LogFileName, "w");
		if (LogFile == NULL)
		{
		    printf("WARNING     : Could not open Log File\n");
		    printf("WARNING     : %s.\n", LogFileName);
		    return -1;
		}
		fclose(LogFile);
		tofile = true;
	    }
	    else 
		tofile = false;
	    return 0;
	};

    /// Print a plain message to the log file only
    int filemessage(const char *msg, ...) const
	{
	    if (mode >= 0)
	    {
		if (tofile)
		{
		    va_list ap;
		    va_start(ap, msg);
		    FILE* LogFile = fopen(LogFileName, "a");
		    fprintf(LogFile, "      ");
		    vfprintf(LogFile, msg, ap);
		    fprintf(LogFile, "\n");
		    fflush(LogFile);
		    fclose(LogFile);
		    va_end(ap);
		}
	    }
	    return 0;
	}

    /// Print a plain message 
    int message(const char *msg, ...) const
	{
	    if (mode >= 0)
	    {
		char message_buffer[MAX_MESSAGE_LENGTH];
		va_list ap;
		va_start(ap, msg);
		printf("      ");
		vsnprintf(message_buffer, MAX_MESSAGE_LENGTH, msg, ap);
		printf(message_buffer);
		printf("\n");
		fflush(stdout);
		if (tofile)
		{
		    va_start(ap, msg);
		    FILE* LogFile = fopen(LogFileName, "a");
		    fprintf(LogFile, "      ");
		    vfprintf(LogFile, msg, ap);
		    fprintf(LogFile, "\n");
		    fflush(LogFile);
		    fclose(LogFile);
		}
		va_end(ap);
	    }
	    return 0;
	}

    /// Continue a plain message
    int continuesmessage(const char *msg, ...) const
	{
	    if (mode >= 0)
	    {
		char message_buffer[MAX_MESSAGE_LENGTH];
		va_list ap;
		va_start(ap, msg);
		vsnprintf(message_buffer, MAX_MESSAGE_LENGTH, msg, ap);
		printf(message_buffer);
		fflush(stdout);
		if (tofile)
		{
		    va_start(ap, msg);
		    FILE* LogFile = fopen(LogFileName, "a");
		    //fprintf(LogFile, "      ");
		    vfprintf(LogFile, msg, ap);
		    fflush(LogFile);
		    fclose(LogFile);
		}
		va_end(ap);
	    }
	    return 0;
	}

    /// Print a tic 
    int tic() const
	{
	    if (mode >= 0)
	    {
		printf(".");
		fflush(stdout);
	    }
	    return 0;
	}

    /// Print a log message 
    int logging(const char *msg, ...) const
	{
	    if (mode >= 1)
	    {
		va_list ap;
		va_start(ap, msg);
		FILE* LogFile = stdout; //(LogFileName, "a");
		if (tofile)
		    LogFile = fopen(LogFileName, "a");
		fprintf(LogFile, "LOGGING     : ");
		vfprintf(LogFile, msg, ap);
		fprintf(LogFile, "\n");
		va_end(ap);
		fflush(LogFile);
		if (tofile)
		    fclose(LogFile);
	    }
	    return 0;
	}

    /// Prelude for a fatal error message 
    int fatalerrorloggingpre(const char* file, const int line) const
	{
	    if (mode >= 0)
	    {
		printf("\n\n!! ----------------------------------------- \n");
		printf("!!  FATAL ERROR in %s:%i\n", file, line);
		if (tofile)
		{
		    FILE* LogFile = fopen(LogFileName, "a");
		    fprintf(LogFile, "\n\n!! ----------------------------------------- \n");
		    fprintf(LogFile, "!!  FATAL ERROR in %s:%i\n", file, line);
		    fclose(LogFile);
		}
		return 1;
	    }
	    return 0;
	}

    /// Print a fatal error message 
    int fatalerrorlogging(const char *msg, ...) const
	{
	    if (mode >= 0)
	    {
		char message_buffer[MAX_MESSAGE_LENGTH];
		va_list ap;
		va_start(ap, msg);
		printf("!!  FATAL ERROR : ");
		vsnprintf(message_buffer, MAX_MESSAGE_LENGTH, msg, ap);
		printf(message_buffer);
		printf( "\n");
		fflush(stdout);
		if (tofile)
		{
		    va_start(ap, msg);
		    FILE* LogFile = fopen(LogFileName, "a");
		    fprintf(LogFile, "!!  FATAL ERROR : ");
		    vfprintf(LogFile, msg, ap);
		    fprintf(LogFile, "\n");
		    fflush(LogFile);
		    fclose(LogFile);
		}
		va_end(ap);
	    }
	    return 0;
	}

    /// Prelude for a warning 
    int warningpre(const char* file, const int line) const
	{
	    if (mode >= 0)
	    {
		printf("\n\n!! ----------------------------------------- \n");
		printf("!!  WARNING     in %s:%i\n", file, line);
		if (tofile)
		{
		    FILE* LogFile = fopen(LogFileName, "a");
		    fprintf(LogFile, "\n\n!! ----------------------------------------- \n");
		    fprintf(LogFile, "!!  WARNING     in %s:%i\n", file, line);
		    fclose(LogFile);
		}
		return 1;
	    }
	    return 0;
	}
 
    /// Print a warning 
   int warning(const char *msg, ...) const
	{
	    if (mode >= 0)
	    {
		char message_buffer[MAX_MESSAGE_LENGTH];
		va_list ap;
		va_start(ap, msg);
		printf("!!  WARNING    : ");
		vsnprintf(message_buffer, MAX_MESSAGE_LENGTH, msg, ap);
		printf(message_buffer);
		printf("\n");
		fflush(stdout);
		if (tofile)
		{
		    va_start(ap, msg);
		    FILE* LogFile = fopen(LogFileName, "a");
		    fprintf(LogFile, "!!  WARNING     : ");
		    vfprintf(LogFile, msg, ap);
		    fprintf(LogFile, "\n");
		    fflush(LogFile);
		    fclose(LogFile);
		}
		va_end(ap);
	    }
	    return 0;
	}
 
   /// Prelude for a user error 
    int usererrorloggingpre(const char* file, const int line) const
	{
	    if (mode >= 0)
	    {
		printf("\n\n!! ----------------------------------------- \n");
		printf("!!  USER ERROR  in %s:%i\n", file, line);
		if (tofile)
		{
		    FILE* LogFile = fopen(LogFileName, "a");
		    fprintf(LogFile, "\n\n!! ----------------------------------------- \n");
		    fprintf(LogFile, "!!  USER ERROR  in %s:%i\n", file, line);
		    fclose(LogFile);
		}
		return 1;
	    }
	    return 0;

	}

    /// Print a user error 
    int usererrorlogging(const char *msg, ...) const
	{
	    if (mode >= 0)
	    {
		char message_buffer[MAX_MESSAGE_LENGTH];
		va_list ap;
		va_start(ap, msg);
		printf("!!  USER ERROR  : ");
		vsnprintf(message_buffer, MAX_MESSAGE_LENGTH, msg, ap);
		printf(message_buffer);
		printf("\n");
		fflush(stdout);
		if (tofile)
		{
		    va_start(ap, msg);
		    FILE* LogFile = fopen(LogFileName, "a");
		    fprintf(LogFile, "!!  USER ERROR  : ");
		    vfprintf(LogFile, msg, ap);
		    fprintf(LogFile, "\n");
		    fflush(LogFile);
		    fclose(LogFile);
		}
		va_end(ap);
	    }
	    return 0;
	}

    /// Print a debug message 
    int debuglogging(const char *msg, ...) const
	{
	    if (mode >= 3)
	    {
		va_list ap;
		va_start(ap, msg);
		FILE* LogFile = stdout; //(LogFileName, "a");
		if (tofile)
		    LogFile = fopen(LogFileName, "a");
		fprintf(LogFile, "DEBUG       : ");
		vfprintf(LogFile, msg, ap);
		fprintf(LogFile, "\n");
		va_end(ap);
		fflush(LogFile);
		if (tofile)
		    fclose(LogFile);
	    }
	    return 0;
	}

    /// Prelude for a debug error message 
    int debugerrorloggingpre(const char* file, const int line) const
	{
	    if (mode >= 2)
	    {
		printf("\n\n!! ----------------------------------------- \n");
		printf("!! DEBUG ERROR in %s:%i\n", file, line);
		if (tofile)
		{
		    FILE* LogFile = fopen(LogFileName, "a");
		    fprintf(LogFile, "\n\n!! ----------------------------------------- \n");
		    fprintf(LogFile, "!! DEBUG ERROR in %s:%i\n", file, line);
		    fclose(LogFile);
		}
		return 1;
	    }
	    return 0;
	}

    /// Print a debug error message 
    int debugerrorlogging( const char *msg, ...) const
	{
	    if (mode >= 2)
	    {
		char message_buffer[MAX_MESSAGE_LENGTH];
		va_list ap;
		va_start(ap, msg);
		printf("!! DEBUG ERROR : ");
		vsnprintf(message_buffer, MAX_MESSAGE_LENGTH, msg, ap);
		printf(message_buffer);
		printf("\n");
		fflush(stdout);
		if (tofile)
		{
		    va_start(ap, msg);
		    FILE* LogFile = fopen(LogFileName, "a");
		    fprintf(LogFile, "!! DEBUG ERROR : ");
		    vfprintf(LogFile, msg, ap);
		    fprintf(LogFile, "\n");
		    fflush(LogFile);
		    fclose(LogFile);
		}
		va_end(ap);
	    }
	    return 0;
	}

    /// Be quiet 
    inline int quiet( const char *msg, ...) const
	{
	    return 0;
	}

// public attributes
 public:
  static int mode;

// private methods
 private:

// private attributes
 private:
    static bool tofile;
    static char LogFileName[199];
};

#endif

