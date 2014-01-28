/*  @brief Typical Message used for communication
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *  Created on: April 20, 2010
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "PSProSockets.h"

// TODO raus
#include "PSProLogging.h"

namespace PSProMigIF
{
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

      // help functions
      static Message* generateMsg(unsigned long _ulCustomSize = 0)
      {
        const unsigned long ulMessageSize = size(m_gCurrentVersion, _ulCustomSize);

        Message* const pMessage = (Message*) malloc(ulMessageSize);
        pMessage->m_nMessageVersion = m_gCurrentVersion;
        pMessage->m_ulMessageSize = ulMessageSize;
        pMessage->m_ulCustomDataSize = _ulCustomSize;
        pMessage->m_ulCostumDataPos = headerSize(m_gCurrentVersion);

        INFO_PRINTF(PSPRO_LOGGER, "%lu", ulMessageSize);

        return pMessage;
      }

      static unsigned long headerSize(int32_t _nVersion)
      {
        unsigned long ulHeaderSize = 0;

        switch(_nVersion)
        {
          // current version
          default:
          {
            ulHeaderSize = sizeof(Message);
            break;
          }
        }

        return ulHeaderSize;
      }

      static unsigned long size(int32_t _nVersion, unsigned long _ulCustomSize)
      {
        return (headerSize(_nVersion) + _ulCustomSize);
      }

      inline char* getCostumPtr(void)
      {
        return &((char*)(this))[m_ulCostumDataPos];
      }

      inline void sendMsg(PSProSocket* const _pSocket)
      {
        _pSocket->sendBuffer((char*)this, m_ulMessageSize);
      }

      static Message* recvMsg(PSProSocket* const _pSocket)
      {
        // receive message size
        unsigned long ulMessageSize = 0;
        _pSocket->recvBuffer((char*)&ulMessageSize, sizeof(unsigned long));

        INFO_PRINTF(PSPRO_LOGGER, "ulMessageSize %lu", ulMessageSize);

        unsigned long ulCustomDataSize = 0;
        _pSocket->recvBuffer((char*)&ulCustomDataSize, sizeof(unsigned long));

        INFO_PRINTF(PSPRO_LOGGER, "ulCustomDataSize %lu", ulCustomDataSize);

        // allocate memory
        Message* const pMessage = generateMsg(ulCustomDataSize);

        // receive message
        const int nReturnValue =_pSocket->recvBuffer((char*) (((char*)pMessage) + 2*sizeof(unsigned long)), ulMessageSize
            - 2*sizeof(unsigned long));

        // check sizes
        if(nReturnValue < 0)
        {
          free(pMessage);
          return NULL;
        }

        return pMessage;
      }

      /*! @brief Message version
       *  If the header changes, change the version number
       */
      static const int32_t m_gCurrentVersion = 1;
  };
}

#endif
