/*  @brief This file contains several server settings
 */

#ifndef SERVERSETTINGS_HPP_
#define SERVERSETTINGS_HPP_

#include <string>

namespace PSProMigIF
{
  /*! @return Character buffer; default size
   */
  #define STRING_SIZE 1024

  /*! @brief Welcome message from master: ready
   */
  inline std::string backendMasterReadyMessage(void)
  {
    return std::string("(Master) Ready...");
  }

  /*! @brief Welcome message from slave: ready
   */
  inline std::string backendSlaveMessage(void)
  {
    return std::string("(Slave) Don't connect to me...");
  }

  /*! @brief Welcome message from master: running as
   */
  inline std::string backendMasterBusyMessage(void)
  {
    return std::string("(Master) Running as ");
  }

  /*! @brief Local host string
   */
  inline std::string localhost(void)
  {
    return std::string("localhost");
  }

  /*! @brief Define connection port between server and client
   *  @return Returns the connection port of the server connection
   */
  inline unsigned int serverConnectionPort(void)
  {
    return 55555;
  }

  /*! @brief Define connection port between server and client
   *  @return Returns the connection port of the server connection
   */
  inline unsigned int daemonConnectionPort(void)
  {
    return 26698;
  }

  /*! @brief Define connection port between server and client
   *  @return Returns the connection port of the server connection
   */
  inline unsigned int remoteFileEngingPort(void)
  {
    return 26699;
  }

  /*! @return Character buffer; default size
   */
  inline unsigned int stringSize(void)
  {
    return 1024;
  }

  /*! @@return Receive time out during initialization (in sec)
   */
  inline unsigned int recvTimeoutInit(void)
  {
    return 10;
  }
}





#endif
