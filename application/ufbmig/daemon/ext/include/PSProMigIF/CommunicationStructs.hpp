/*! @brief This class contains structs used by the server communication
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *  Created on: April 21, 2010
 */

#ifndef COMMUNICATIONSTRUCTS_H_
#define COMMUNICATIONSTRUCTS_H_

#include <string>

#include "ServerSettings.hpp"

namespace PSProMigIF
{
  /*! @brief This struct holds all information which are necessary to start the server application
   */
  struct StartupInfo
  {
      StartupInfo() :
        m_nWaitForConnectionTimeout(30) ,
        m_nConnectToTimeout(30) // default value
      {
        sprintf(m_sHostname, "%s", localhost().c_str());
        sprintf(m_sSSHHostname, "%s", localhost().c_str());
        sprintf(m_sUsername, "%s", "");
        sprintf(m_sPassword, "%s", "");
        sprintf(m_sBinaryPath, "%s", "");
        m_uPort = serverConnectionPort();
        m_uDaemonPort = daemonConnectionPort();
        m_uFileEnginePort = remoteFileEngingPort();
        m_uRemotePort = m_uPort;
        m_uRemoteDaemonPort = m_uDaemonPort;
        m_uRemoteFileEnginePort = m_uFileEnginePort;
      }

      char m_sHostname[1024];
      char m_sSSHHostname[1024];
      char m_sUsername[1024];
      char m_sPassword[1024];
      char m_sBinaryPath[1024];
      unsigned int m_uPort;
      unsigned int m_uDaemonPort;
      unsigned int m_uFileEnginePort;
      unsigned int m_uRemotePort;
      unsigned int m_uRemoteDaemonPort;
      unsigned int m_uRemoteFileEnginePort;
      int m_nWaitForConnectionTimeout;
      int m_nConnectToTimeout;
  };

  /*! @brief Holds informations from the license server
   */
  struct LicenseServerInfo
  {
      char m_sLicenseKey[64]; // license key from the license server
      int m_nValidityTime; // validity time of the license (in days)
  };

  /*! @brief Enum which holds the error codes of the startup procedure
   */
  enum StartServerError
  {
    StartServer_CannotPingDaemon = 1,
    StartServer_AuthentificationFailed = 2,
    StartServer_InfinibandTestFailed = 3,
    StartServer_Pv4DaemonNotAvailable = 4,
    StartServer_LicenseServerNotAvailable = 5,
    StartServer_LicenseExpired = 6,
    StartServer_StartBinaryFailed = 7,
    StartServer_NoMasterNode = 8,
    StartServer_BinaryAlreadyRunning = 9,
    StartServer_GotInvalidSocket = 10,
    StartServer_BadBinary = 11,
    StartServer_UnknownProblem = 12,
    StartServer_ServerAlreadyStarted = 13,
    StartServer_ServerAlreadyConnected = 14,
    StartServer_ServerNotStarted = 15,
    StartServer_ServerAlreadyDisconnected = 16,
    StartServer_CannotStartServerStateControl = 17,
    StartServer_ServerStateControlAlreadyStarted = 18,
    StartServer_ServerCommunicationError = 19,
    StartServer_CannotConnect2Daemon = 20,
    StartServer_PSProDaemon_UnknownHost = 21,
    StartServer_PSProDaemon_CannotPingDaemon = 22,
    StartServer_PSProDaemon_SslFailed = 23,
    StartServer_SSHTunnelFailed = 24,
    StartServer_ServerNamesDoNotMatch = 25,
    StartServer_UnsupportedInterface = 26,
    StartServer_PortAlreadyInUse = 27,

    StartServer_NoError = 255
  };

  /*! @brief Server states
   */
  enum ServerState
  {
    ServerState_Started, // running + connected
    ServerState_Disconnected, // running + disconnected
    ServerState_Stopped,
    ServerState_StartedButNotUnderControl
  // running + connected
  // not running
  };

  /*! @brief Converts the ServerState enum to a string name
   *  @param _serverState The server state
   *  @return The string conversion
   */
  inline std::string serverStateToStr(const ServerState& _serverState)
  {
    switch(_serverState)
    {
      case ServerState_Started:
      {
        return std::string("Running + Connected");
      }
      case ServerState_Disconnected:
      {
        return std::string("Running + Disconnected");
      }
      case ServerState_Stopped:
      {
        return std::string("Not Running");
      }
    case ServerState_StartedButNotUnderControl:
      // just ignore that state, is this the correct behavior?
      ;
    }

    return std::string("Unknown Server State");
  }

  /*! @brief Server stop reason
   *  Just useful in the case that the server is not running
   */
  enum ServerStopReason
  {
    ServerStopReason_normal,
    ServerStopReason_crash
  };

  /*! @brief Server working state
   *  Just useful in the case that the server is running
   */
  enum ServerWorkingState
  {
    ServerWorkingState_idle, // nothing to do
    ServerWorkingState_busy
  // busy; for example processing
  };

  /*! @brief Converts the ServerState enum to a string name
   *  @param _serverState The server state
   *  @return The string conversion
   */
  inline std::string serverWorkingStateToStr(const ServerWorkingState& _serverWorkingState)
  {
    switch(_serverWorkingState)
    {
      case ServerWorkingState_idle:
      {
        return std::string("Idle");
      }
      case ServerWorkingState_busy:
      {
        return std::string("Busy");
      }
    }

    return std::string("Unknown Server Working State");
  }

  /*! @brief Enum which holds the error codes of the server communication
   */
  enum ServerStateControlError
  {
    ServerStateControl_ConnectionFailed = 1,
    ServerStateControl_CouldNotBindSocket = 2,
    ServerStateControl_ServerListenProblem = 3,
    ServerStateControl_GotInvalidSocket = 4,
    ServerStateControl_BadBinary = 5,
    ServerStateControl_UnknownProblem = 6,
    ServerStateControl_NoMasterNode = 7,
    ServerStateControl_BinaryAlreadyRunning = 8,
    ServerStateControl_CouldNotStartControlThread = 9
  };

  /*! @brief Struct which holds connection information
   */
  struct ConnectionInfo
  {
      ConnectionInfo() :
        m_nConnectToTimeout(30), // default value
            m_nWaitForConnectionTimeout(30)
      {
        sprintf(m_sHostname, "%s", localhost().c_str());
        m_uPort = serverConnectionPort();
        m_uRemotePort = m_uPort;
      }

      char m_sHostname[1024];
      unsigned int m_uPort;
      unsigned int m_uRemotePort;

      int m_nConnectToTimeout; // in seconds
      int m_nWaitForConnectionTimeout; // in seconds
  };

}

#endif
