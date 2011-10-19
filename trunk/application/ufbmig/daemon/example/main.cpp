/*! @file main.cpp for building the PSProMigServerIF
 *  @brief main.cpp for building the PSProMigServerIF
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *  Created on: Mar 23, 2010
 *  Modified: Alexander Petry (ITWM), 11.10.2011
 *  */

#include <cstdio>
#include <cstring>
#include <string>
#include <cstdlib>

#include "PSProLogging.h"
#include "ExternalServerInterfaceLogging.h"
#include "ServerCommunication.h"
#include "Server.h"
#include "Message.h"
#include "ServerSettings.hpp"

int main(int _nArgc, char* _pcArgv[])
{
  const std::string server_app_name("UfBMig (SDPA)");
  const std::string server_app_version("0.0.1");

  // init server logging
  initializeBackendFileLogging();

  // start server communication
  PSProMigIF::StartServer::registerInstance
    ( server_app_name
    , new PSProMigIF::ServerHelper(server_app_version)
    );

  PSProMigIF::StartServer* pStartServer
    (PSProMigIF::StartServer::getInstance(server_app_name));

  pStartServer->handleExceptionsByLibrary(false);

  pStartServer->addCommunication
    (new PSProMigIF::ServerCommunicationListen());

  try
  {
    // start server control object
    pStartServer->start();
  }
  catch (PSProMigIF::StartServer::StartServerException const &ex)
  {
    std::cerr << "error starting server: " << ex.what();
    return EXIT_FAILURE;
  }

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

    // doing calculations
    pStartServer->busy();

    sleep(1);

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
