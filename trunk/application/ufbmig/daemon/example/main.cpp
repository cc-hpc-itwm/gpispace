/*! @file main.cpp for building the PSProMigServerIF
 *  @brief main.cpp for building the PSProMigServerIF
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *  Created on: Mar 23, 2010
 *  */

#include "centralDefines.h"
#include "centralDefinesKeywords.h"
#include <cstdio>
#include <cstring>
#include <string>

#include "PSProLogging.h"
#include "PSProMigVersion.h"
#include "ServerCommunication.h"
#include "ExternalServerInterfaceLogging.h"
#include "Server.h"
#include "Message.h"
#include "SignalHandler.h"
#include "ServerSettings.hpp"

#include <QString>

int main(int _nArgc, char* _pcArgv[])
{
  // init server logging
  initializeBackendFileLogging();

  signal(SIGSEGV, SignalHandler::signal_handler_segfault);
  signal(SIGBUS, SignalHandler::signal_handler_segfault);
  signal(SIGABRT, SignalHandler::signal_handler_segfault);

  // Welcome
  INFO_PRINTF(PSPRO_LOGGER, "This is " PROJECT_NAME " " BACKEND " %s", PSPROMIG_VERSION_STRING);

  // start server communication
  PSProMigIF::StartServer::registerInstance("Interactive Migration", new PSProMigIF::ServerHelper(std::string("0.7.1")));

  PSProMigIF::StartServer* pStartServer = PSProMigIF::StartServer::getInstance("Interactive Migration");

  pStartServer->addCommunication(new PSProMigIF::ServerCommunicationListen());

  // start server control object
  pStartServer->start();

  pStartServer->idle();

  // sending waiting for init
  PSProMigIF::Message* pMessage = PSProMigIF::Message::generateMsg(0);
  pMessage->m_nCommand = 0;
  pMessage->sendMsg(pStartServer->communication());

  sleep(1);

  // getting command to init
  pMessage = PSProMigIF::Message::recvMsg(pStartServer->communication());

  sleep(1);

  // sending doing init
  pMessage = PSProMigIF::Message::generateMsg(0);
  pMessage->m_nCommand = 1;
  pMessage->sendMsg(pStartServer->communication());

  sleep(3);

  // sending init successful
  pMessage = PSProMigIF::Message::generateMsg(0);
  pMessage->m_nCommand = 2;
  pMessage->sendMsg(pStartServer->communication());

  sleep(1);

  while(true)
  {
    // waiting for data for calculations
    PSProMigIF::Message* pMessage2 = PSProMigIF::Message::recvMsg(pStartServer->communication());
    sleep(1);

    // doing calculations
    pStartServer->busy();

    pMessage = PSProMigIF::Message::generateMsg(0);
    pMessage->m_nCommand = 4;
    pMessage->sendMsg(pStartServer->communication());

    sleep(3);

    // sending successful + result
    pMessage = PSProMigIF::Message::generateMsg(pMessage2->m_ulCustomDataSize);
    memcpy(pMessage->getCostumPtr(), pMessage2->getCostumPtr(), pMessage2->m_ulCustomDataSize);
    pMessage->m_nCommand = 5;
    pMessage->sendMsg(pStartServer->communication());

    // doing calculations
    pStartServer->idle();
  }

  // shutdown server
  pStartServer->stop();
}
