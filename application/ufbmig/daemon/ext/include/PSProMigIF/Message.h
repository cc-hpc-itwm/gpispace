/*  @brief Typical Message used for communication
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *  Created on: April 20, 2010
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <stdio.h>
#include <stdint.h>

class PSProSocket;

namespace PSProMigIF
{
  class ServerCommunication;

  struct Message
  {
      // header
      /*! @brief Length of the message
       */
      unsigned long m_ulMessageSize;

      /*! @brief Length of the message
       */
      unsigned long m_ulCustomDataSize;

      /*! @brief Message version
       *  Length: 4 byte
       */
      int32_t m_nMessageVersion;

      /*! @brief Position of custom data
       */
      unsigned long m_ulCostumDataPos;

      /*! @brief Message command
       */
      int32_t m_nCommand;

      /*! @brief Generates a message where the costum part has a size of n bytes
       *  @param _ulCustomSize Size of the custom part
       *  @return Pointer to the generated message
       */
      static Message* generateMsg(unsigned long _ulCustomSize = 0);

      /*! @return Returns the size of the header for a given version number
       *  @param _nVersion The version
       */
      static unsigned long headerSize(int32_t _nVersion);

      /*! @return Returns the size of the message for a given version number
       *  @param _nVersion The version
       *  @param _ulCustomSize Size of the custom part
       */
      static unsigned long size(int32_t _nVersion, unsigned long _ulCustomSize);

      /*! @return Returns the pointer to the custom part in the message
       */
      char* getCostumPtr(void);

      /*! @brief Function for sending a message over a given communication channel
       *  @param _pServerCommunication The communication channel
       */
      void sendMsg(ServerCommunication* const _pServerCommunication, const int _nTimeout=-1);

      /*! @brief Function for sending a message over a given communication channel
       *  @param _pSocket The communication channel
       */
      void sendMsg(PSProSocket* const _pSocket, const int _nTimeout=-1);

      /*! @brief Function for receiving a message over a given communication channel
       *  @param _pSocket The communication channel
       *  @return Pointer to the generated message
       */
      static Message* recvMsg(PSProSocket* const _pSocket, const int _nTimeout=-1);

      /*! @brief Function for receiving a message over a given communication channel
       *  @param _pServerCommunication The communication channel
       *  @return Pointer to the generated message
       */
      static Message* recvMsg(ServerCommunication* const _pServerCommunication, const int _nTimeout=-1);

      /*! @brief Message version
       *  If the header changes, change the version number
       */
      static int32_t currentVersion(void);
  };
}

#endif
