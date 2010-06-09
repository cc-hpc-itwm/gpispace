/***************************************************************************
                          tracememhandler.cpp  -  description
                             -------------------
    begin                : Thu Feb 23 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

#include "tracememhandler.h"

TraceMemHandler::TraceMemHandler(char* MemPtr, FILE_MODE _F_MODE)
{
    endianess = TestByteOrder();
  F_MODE = _F_MODE;
  MemInput = MemPtr;
  MemInputBegin = MemInput;

  switch(F_MODE)
  {
    case SEGY_BIGENDIAN:
    {
      SegYHeader* Header = (SegYHeader*) MemInput;

      unsigned short ns_tmp = Header->ns;
      unsigned short dt_tmp = Header->dt;
      if (endianess != BIGENDIAN)
      {
	  swap_bytes(&(ns_tmp), 1, sizeof(short));
	  swap_bytes(&(dt_tmp), 1, sizeof(short));
      }
      Nt = ns_tmp;
      dt = dt_tmp / 1000.0 / 1000.0;
      T0 = 0;

      break;
    }
    case RAW_FLOAT:
    {
      RawFloatHeader* Header = (RawFloatHeader*) MemInput;

      dt = Header->fileInfo[14] / 1000.0 / 1000.0;
      T0 = Header->userParam[7] / 1000.0;
      Nt = (Header->userParam[8] - Header->userParam[7] + Header->fileInfo[14]/1000)/ (Header->fileInfo[14]/1000);
      break;
    }

    default:
      std::cout << "ERROR in TraceMemHandler::TraceMemHandler: Illegal TRACEFILE_MODE\n";
      break;
  };

  tracl = 0;

  
  
  
  
}

TraceMemHandler::~TraceMemHandler(){
}

int TraceMemHandler::Next()
{
  switch(F_MODE)
  {
    case SEGY_BIGENDIAN:
    {
      SegYHeader* Header = (SegYHeader*) MemInput;
      MemInput += sizeof(SegYHeader);

      unsigned short ns_tmp = Header->ns;
      unsigned short dt_tmp = Header->dt;
      if (endianess != BIGENDIAN)
      {
	  swap_bytes(&(ns_tmp), 1, sizeof(short));
	  swap_bytes(&(dt_tmp), 1, sizeof(short));
      }
      Nt = ns_tmp;
      dt = dt_tmp / 1000.0 / 1000.0;
      T0 = 0;
      break;
    }
    case RAW_FLOAT:
    {
      // nothing to do for RAW_FLOAT
      break;
    }

    default:
      std::cout << "ERROR in TraceMemHandler::ReadTrace: Illegal TRACEFILE_MODE\n";
      break;
  };

#ifdef DEBUG_LOG
  std::cout << "TraceMemHandler::Next: Read from next trace:" << std::endl;
  std::cout << "                  T0 = " << T0 << std::endl;
  std::cout << "                  dt = " << dt << std::endl;
  std::cout << "                  Nt = " << Nt << std::endl;
#endif

  return 0;
}

void TraceMemHandler::ReadTraceData(float* Trace)
{
  switch(F_MODE)
  {
    case SEGY_BIGENDIAN:
    {
      memcpy((char*) Trace, MemInput, Nt*sizeof(float));
      MemInput += Nt*sizeof(float);

      if (endianess != BIGENDIAN)
      {
	  swap_bytes(&Trace[0], Nt, sizeof(float));
      }
      break;
    }  
    case RAW_FLOAT:
    {
      memcpy((char*) Trace, MemInput, Nt*sizeof(float));
      MemInput += Nt*sizeof(float);
      break;
    } 

    default:
      std::cout << "ERROR in TraceMemHandler::ReadTrace: Illegal TRACEFILE_MODE\n";
      break;
  };

  tracl++;
}

void TraceMemHandler::Rewind()
{
  MemInput = MemInputBegin;
  tracl = 0;
}

void TraceMemHandler::Rewind(const int N)
{
  long long N_tot = 0;
  if (F_MODE == RAW_FLOAT)
    N_tot = -N*(Nt*sizeof(float));
  else if (F_MODE == SEGY_BIGENDIAN)
    N_tot = -N*(Nt*sizeof(float) + sizeof(SegYHeader));

  MemInput -= N_tot;
  tracl += N;

  //std::cout << FInput.tellg() << std::endl;
//  FInput.seekg(0, std::ios::beg);
}

int TraceMemHandler::getTracesize()
{
  int Tracesize(0);
  switch(F_MODE)
  {
    case SEGY_BIGENDIAN:
    {
      Tracesize = sizeof(SegYHeader) + Nt*sizeof(float);
      break;
    }
    case RAW_FLOAT:
    {
      Tracesize = sizeof(RawFloatHeader) + Nt*sizeof(float);
      break;
    } 

    default:
      std::cout << "ERROR in TraceMemHandler::ReadTrace: Illegal TRACEFILE_MODE\n";
      break;
  };

  return Tracesize;
  
}
