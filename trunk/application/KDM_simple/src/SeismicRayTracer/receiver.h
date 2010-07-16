/***************************************************************************
                          receiver.h  -  description
                             -------------------                          */
/**
   Base class for the receiver. A receiver consists of a position, a
   buffer for the recorded signals and a flag for (in-) activity.
   The policy for which signal to record has still to be implemented.
  *@author Dirk Merten
  */

/*                           -------------------                         

    begin                : Wed Jan 25 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef RECEIVER_H
#define RECEIVER_H


#include "structures/recsig.h"
#include "structures/point3d.h"
#include <math.h>

const int g_MAXSIG = 2;

class Receiver {
public: //Public methods
  /// Standard constructor to initialize the receiver as inactive.
  Receiver():pos(0,0,0), LastTstep(-1),active(false),pSignal(&(Signals[g_MAXSIG-1])){};
  /// Initialize the receiver to the given pos and as active.
  Receiver(const point3D<float> & _pos):pos(_pos),LastTstep(-1),
      active(true),pSignal(&(Signals[g_MAXSIG-1])){};
  Receiver (const Receiver& );
  ~Receiver(){};
  /// Store the given signal in the local buffer
  /** If a signal with the same time step Tstep has allready
      been saved, the signal with the lower arrival time is stored.
      If the buffer is full a warning will be given */
  bool SaveSig(const RecSig& ThisSig, const int& Tstep);
  /// Clear the buffer of signals 
  void clear();
  Receiver& operator = (const Receiver& );
  /// Return a pointer to the last recorded Signal
  RecSig* GetSig();

  float get_ttime();
  float get_ttime(float& init_x, float& init_y);

private: // Private methods
  /// Stores the local signal buffer in a remote memory space and frees the local buffer
  bool StoreSigBuffer();

  /// investigate pSignal and return the resulting RecSig*
  RecSig* Get_firstarrival();

public: // Public attributes
  /// Position of the receiver 
  point3D<float> pos;
  /// Tstep of the last signal that has been received
  short LastTstep;
  /// Flag if receiver is active (true) or not.
  bool active;
  
private: // Private attributes
  /// Pointer to the next local memory to store signal
  RecSig * pSignal;
  /// Local list of signals received 
  RecSig Signals [g_MAXSIG];
  friend class OutputClass;
  friend class TTFileHandler;
};

#endif
