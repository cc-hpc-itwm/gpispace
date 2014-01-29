/*! @brief This file contains predefined signal handler
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *  Created on: May 05, 2010
 *  */

#ifndef SIGNALHANDLER_H_
#define SIGNALHANDLER_H_

#include <signal.h>

namespace SignalHandler
{
  void init(void);

  /*! @brief Starts a signal exit timer
   *  When the timeout is over the sigalarm signal will be sent
   *  @param _nTimeoutSecs The timeout in seconds
   */
  void startExitSignalTimer(int _nTimeoutSecs);

  /*! @brief This signal handler makes an exit with value -1
   */
  void signal_handler_sigalarm(int);

  /*! @brief This signal handler is implemented for segmentation fault.
   *  The signal handler will try to print a backtrace to console
   */
  void signal_handler_segfault(int);

  /* @brief For testing segmentation fault handling
   */
  void generateSegfault(void);

  /* @brief For testing bus error handling
   */
  void generateBusError(void);
}



#endif
