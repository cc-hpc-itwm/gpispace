#ifndef SERVERLOGGING_H
#define SERVERLOGGING_H

#include <stdint.h>

/// Initialize the logging (based on log4cplus) on the backend over TCP to the frontend node
void initializeBackendNetworkLogging(int index, int32_t ip, int32_t port);

/// Initialize the logging (based on log4cplus) on the backend to a file
void initializeBackendFileLogging();

#endif
