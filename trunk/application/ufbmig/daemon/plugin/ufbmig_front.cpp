#include "ufbmig_front.hpp"

#include <errno.h>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include <cstdio>
#include <cstring>
#include <string>
#include <cstdlib>
#include <iostream>

#include "ServerCommunication.h"
#include "Server.h"
#include "Message.h"
#include "ServerSettings.hpp"

static const std::string SERVER_APP_NAME("UfBMig (SDPA)");
static const std::string SERVER_APP_VERS("0.0.1");

class UfBMigFrontImpl : FHG_PLUGIN
                      , public ufbmig::Frontend
{
public:
  UfBMigFrontImpl ()
  {
    // start server communication
    PSProMigIF::StartServer::registerInstance
      ( SERVER_APP_NAME
      , new PSProMigIF::ServerHelper(SERVER_APP_VERS)
      );

    m_server = PSProMigIF::StartServer::getInstance(SERVER_APP_NAME);
    m_server->handleExceptionsByLibrary(false);
    m_server->addCommunication (new PSProMigIF::ServerCommunicationListen());
  }

  FHG_PLUGIN_START()
  {
    try
    {
      // start server control object
      m_server->start();
    }
    catch (PSProMigIF::StartServer::StartServerException const &ex)
    {
      LOG(ERROR, "could not start server connection: " << ex.what());
      FHG_PLUGIN_FAILED(ETIMEDOUT);
      // TODO:
      //   mark plugin as incomplete, try to start connection again...
    }

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    m_server->stop();
    FHG_PLUGIN_STOPPED();
  }

  int initialize()
  {
    return 0;
  }

  int calculate()
  {
    return 0;
  }

  int finalize()
  {
    return 0;
  }

  int cancel()
  {
    return 0;
  }

  void initialize_done (int)
  {
    return;
  }

  void calculate_done (int)
  {
    return;
  }

  void finalize_done (int)
  {
    return;
  }

  void cancel_done (int)
  {
    return;
  }
private:
  PSProMigIF::StartServer* m_server;
};

EXPORT_FHG_PLUGIN( ufbmig_front
                 , UfBMigFrontImpl
                 , "provides the frontend functionality for the UfBMig"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "ufbmig_back"
                 , ""
                 );
