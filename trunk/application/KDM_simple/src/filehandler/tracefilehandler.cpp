/***************************************************************************
                          tracefilehandler.cpp  -  description
                             -------------------
    begin                : Thu Feb 23 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

#include "tracefilehandler.h"

TraceFileHandler::TraceFileHandler(const char* Name, FILE_MODE _F_MODE, int& ierr)
{
    endianess = TestByteOrder();
    F_MODE = _F_MODE;
    
    //
    FInput.open(Name, std::ios::binary);
    if (FInput.fail())
    {
	
	ierr = -1;
	return;
    }
    
    // set length of file header and trace header
    switch(F_MODE)
    {
	case SEGY_BIGENDIAN:
	case SEGY_BIGENDIAN_SXFLOAT:
	{
	    LengthOfFileHeader = sizeof(SegYBHeader) + sizeof(SegYEBCHeader);
	    LengthOfTraceHeader = sizeof(SegYHeader);
	    break;
	}
	case SU_LITENDIAN:
	{
	    LengthOfFileHeader = 0;
	    LengthOfTraceHeader = sizeof(SegYHeader);
	    break;
	}
	default:
	    
	    break;
    };

    // seek to first trace
    FInput.seekg(LengthOfFileHeader , std::ios::beg);
    if (FInput.fail())
    {
	
	ierr = -1;
	return;
    }

    // read first header
    ierr = ReadHeader();
    if (ierr != 0)
	return;
    if ( CurrentHeader.Nt <= 0 )
    {
	
	ierr = -1;
	return;
    } 

    LengthOfTraceData = CurrentHeader.Nt*sizeof(float);
    LengthOfTrace = LengthOfTraceHeader + LengthOfTraceData;

    // determine total number of traces
    FInput.seekg(0, std::ios::end);
    unsigned long endpos = FInput.tellg();
    TotNofTraces = (endpos - LengthOfFileHeader) / LengthOfTrace;

    // check if trace file contains integer number of traces
    if (endpos != LengthOfFileHeader + TotNofTraces * LengthOfTrace)
    {
	
	
	
	
	
	ierr = -1;
	return;
    }

    if (TotNofTraces > 1)
    {
	FInput.seekg(LengthOfFileHeader + LengthOfTrace , std::ios::beg);
	if (FInput.fail())
	{
	    
	    
	    
	    ierr = -1;
	    return;
	}

	const int Nt_first = CurrentHeader.Nt;
	ierr = ReadHeader();
	if (ierr != 0)
	    return;
	if (CurrentHeader.Nt != Nt_first)
	{
	    
	    
	    
	    
	    ierr = -1;
	    return;
	}
    }

    // rewind to first trace
    FInput.seekg(LengthOfFileHeader, std::ios::beg);

    ierr = 0;
}

TraceFileHandler::~TraceFileHandler(){
  FInput.close();
}

int TraceFileHandler::Next()
{
    return ReadHeader();
}

int TraceFileHandler::ReadTraceData(float* Trace)
{
  switch(F_MODE)
  {
      case SEGY_BIGENDIAN:
      case SEGY_BIGENDIAN_SXFLOAT:
      {
	  FInput.read((char*) &Trace[0], CurrentHeader.Nt*sizeof(float));
	  
	  if (FInput.fail())
	  {
	      
	      return -1;
	  }

	  ibm2float(&Trace[0], CurrentHeader.Nt, endianess);

	  break;
      }  
      case SU_LITENDIAN:
      {
	  FInput.read((char*) &Trace[0], CurrentHeader.Nt*sizeof(float));

	  if (FInput.fail())
	  {
	      
	      return -1;
	  }

	  if (endianess != LITENDIAN)
	    {
	      swap_bytes(&Trace[0], CurrentHeader.Nt, sizeof(float));
	    }
	  break;
      }
      default:
	  
	  return -1;
	  break;
  };

  return 0;
}

int TraceFileHandler::ReadTraces(char* TraceMem, const int Ntr)
{
    FInput.read(TraceMem, Ntr * (sizeof(SegYHeader) + CurrentHeader.Nt*sizeof(float)));
    if (FInput.fail())
    {
	
	return -1;
    }

    switch(F_MODE)
    {
	case SEGY_BIGENDIAN:
	{
	    SegYHeader* Header;
	    float* data;

	    for (int itrace = 0; itrace < Ntr; itrace++)
	    {
		Header = (SegYHeader*) &(TraceMem[itrace * (sizeof(SegYHeader) + CurrentHeader.Nt*sizeof(float))]);

		ConvertSegYHeaderBE(Header);

		data = (float*) &(TraceMem[itrace * (CurrentHeader.Nt*sizeof(float) + sizeof(SegYHeader)) + sizeof(SegYHeader)]);
		ibm2float(data, CurrentHeader.Nt, endianess);
	    }
	    break;
	}
	case SEGY_BIGENDIAN_SXFLOAT:
	{
	    SegYHeader* Header;
	    float* data;
	    for (int itrace = 0; itrace < Ntr; itrace++)
	    {
		Header = (SegYHeader*) &(TraceMem[itrace * (sizeof(SegYHeader) + CurrentHeader.Nt*sizeof(float))]);

		ConvertSegYHeaderBEsxfloat(Header);

		data = (float*) &(TraceMem[itrace * (CurrentHeader.Nt*sizeof(float) + sizeof(SegYHeader)) + sizeof(SegYHeader)]);
		ibm2float(data, CurrentHeader.Nt, endianess);
	    }
	    break;
	}
	case SU_LITENDIAN:
	{
		if (endianess != LITENDIAN)
		  {
		    SegYHeader* Header;
		    float* data;

		    for (int itrace = 0; itrace < Ntr; itrace++)
		      {
			Header = (SegYHeader*) &(TraceMem[itrace * (sizeof(SegYHeader) + CurrentHeader.Nt*sizeof(float))]);

			ConvertSegYHeaderLE(Header);
			
			data = (float*) &(TraceMem[itrace * (CurrentHeader.Nt*sizeof(float) + sizeof(SegYHeader)) + sizeof(SegYHeader)]);
			swap_bytes(data, CurrentHeader.Nt, sizeof(float));
		      }
		  }
	    break;
	}
	default:
	    
	    return -1;
	    break;
    };
    return 0;
}

int TraceFileHandler::ReadTraces(char* TraceMem, const int Ntr, const int N0, const int Ne)
{
    const int Nsample = (CurrentHeader.Nt-N0-Ne);
    for (int itrace = 0; itrace < Ntr; itrace++)
    {
	FInput.read(&TraceMem[itrace * (sizeof(SegYHeader) + Nsample*sizeof(float))], sizeof(SegYHeader));
	FInput.seekg(N0 * sizeof(float),  std::ios::cur);
	FInput.read(&TraceMem[itrace * (sizeof(SegYHeader) + Nsample*sizeof(float)) + sizeof(SegYHeader)], Nsample * sizeof(float));
	FInput.seekg(Ne * sizeof(float),  std::ios::cur);
    }
    if (FInput.fail())
    {
	
	return -1;
    }

    switch(F_MODE)
    {
	case SEGY_BIGENDIAN:
	{
	    SegYHeader* Header;
	    float* data;

	    for (int itrace = 0; itrace < Ntr; itrace++)
	    {
		Header = (SegYHeader*) &(TraceMem[itrace * (sizeof(SegYHeader) + Nsample*sizeof(float))]);

		ConvertSegYHeaderBE(Header);
		  
		data = (float*) &(TraceMem[itrace * (Nsample*sizeof(float) + sizeof(SegYHeader)) + sizeof(SegYHeader)]);
		ibm2float(data, Nsample, endianess);
	    }
	    break;
	}
	case SEGY_BIGENDIAN_SXFLOAT:
	{
	    SegYHeader* Header;
	    float* data;
	    for (int itrace = 0; itrace < Ntr; itrace++)
	    {
		Header = (SegYHeader*) &(TraceMem[itrace * (sizeof(SegYHeader) + Nsample*sizeof(float))]);

		ConvertSegYHeaderBEsxfloat(Header);
		
		data = (float*) &(TraceMem[itrace * (Nsample*sizeof(float) + sizeof(SegYHeader)) + sizeof(SegYHeader)]);
		ibm2float(data, Nsample, endianess);
	    }
	    break;
	}
	case SU_LITENDIAN:
	{
	    SegYHeader* Header;
	    for (int itrace = 0; itrace < Ntr; itrace++)
	    {
		Header = (SegYHeader*) &(TraceMem[itrace * (sizeof(SegYHeader) + Nsample*sizeof(float))]);

		ConvertSegYHeaderLE(Header);
	    }

	    break;
	}
	default:
	    
	    return -1;
	    break;
    };
    return 0;
}

void TraceFileHandler::Rewind()
{
    unsigned long fileoffset = 0;
    if ( (F_MODE ==  SEGY_BIGENDIAN) 
	 || (F_MODE == SEGY_BIGENDIAN_SXFLOAT) )
    {
	fileoffset = sizeof(SegYBHeader) + sizeof(SegYEBCHeader); 
    }
    FInput.seekg(fileoffset, std::ios::beg);
}

void TraceFileHandler::Rewind(const int N)
{
  long long N_tot = -N*(CurrentHeader.Nt*sizeof(float) + sizeof(SegYHeader));
  FInput.seekg(N_tot, std::ios::cur);
}

int TraceFileHandler::NextHeader(briefheader* bHeader)
{
    int ierr = ReadHeader(bHeader);
    if (ierr != 0)
	return ierr;
    FInput.seekg(CurrentHeader.Nt*sizeof(float), std::ios::cur);
    return 0;
}

unsigned long long TraceFileHandler::Seek(const int Itrace)
{
  unsigned long long Pos = LengthOfFileHeader + Itrace*LengthOfTrace;
  FInput.seekg(Pos, std::ios::beg);
  return Pos;
}

int TraceFileHandler::ReadHeader(briefheader* bHeader)
{
    int ierr = ReadHeader();
    *bHeader = CurrentHeader;
    return ierr;
 }

int TraceFileHandler::ReadHeader()
{
  if (FInput.eof())
  {
      
      return -1;
  }

  switch(F_MODE)
  {
    case SEGY_BIGENDIAN:
    {
      SegYHeader Header;
      FInput.read((char*) &Header, sizeof(SegYHeader));

      if (FInput.fail())
      {
        
        return -1;
      }

      ConvertSegYHeaderBE(&Header);

      float scalco = 1.0;
      if (Header.scalco < 0)
	  scalco = 1.0/(-Header.scalco);
      if (Header.scalco > 0)
	  scalco = Header.scalco;

      float scalel = 1.0;
      if ( Header.scalel < 0 )
        scalel = -1.0/Header.scalel;
      if ( Header.scalel > 0 )
        scalel = Header.scalel;

      CurrentHeader.Nt = Header.ns;
      CurrentHeader.dt = Header.dt / 1000.0 / 1000.0;
      CurrentHeader.T0 = Header.delrt / 1000.0;

      CurrentHeader.sx = Header.sx * scalco;
      CurrentHeader.sy = Header.sy * scalco;
      CurrentHeader.gx = Header.gx * scalco;
      CurrentHeader.gy = Header.gy * scalco;
      CurrentHeader.selev = Header.selev * scalel;
      CurrentHeader.gelev = Header.gelev * scalel;

      CurrentHeader.CDP = Header.cdp;
      CurrentHeader.CDPx = (CurrentHeader.sx+CurrentHeader.gx)/2.0;
      CurrentHeader.CDPy = (CurrentHeader.sy+CurrentHeader.gy)/2.0;
      CurrentHeader.Offx = CurrentHeader.gx-CurrentHeader.sx;
      CurrentHeader.Offy = CurrentHeader.gy-CurrentHeader.sy;
      break;
    }
    case SEGY_BIGENDIAN_SXFLOAT:
    {
      SegYHeader Header;
      FInput.read((char*) &Header, sizeof(SegYHeader));

      if (FInput.fail())
      {
        
        return -1;
      }

      ConvertSegYHeaderBEsxfloat(&Header);

      float scalco = 1.0;
      if (Header.scalco < 0)
	  scalco = 1.0/(-Header.scalco);
      if (Header.scalco > 0)
	  scalco = Header.scalco;

      float scalel = 1.0;
      if ( Header.scalel < 0 )
        scalel = -1.0/Header.scalel;
      if ( Header.scalel > 0 )
        scalel = Header.scalel;

      CurrentHeader.Nt = Header.ns;
      CurrentHeader.dt = Header.dt / 1000.0 / 1000.0;
      CurrentHeader.T0 = Header.delrt / 1000.0;

      CurrentHeader.sx = Header.sx * scalco;
      CurrentHeader.sy = Header.sy * scalco;
      CurrentHeader.gx = Header.gx * scalco;
      CurrentHeader.gy = Header.gy * scalco;
      CurrentHeader.selev = Header.selev * scalel;
      CurrentHeader.gelev = Header.gelev * scalel;

      CurrentHeader.CDP = Header.cdp;
      CurrentHeader.CDPx = (CurrentHeader.sx+CurrentHeader.gx)/2.0;
      CurrentHeader.CDPy = (CurrentHeader.sy+CurrentHeader.gy)/2.0;
      CurrentHeader.Offx = CurrentHeader.gx-CurrentHeader.sx;
      CurrentHeader.Offy = CurrentHeader.gy-CurrentHeader.sy;
      break;
    }
    case SU_LITENDIAN:
    {
      SegYHeader Header;
      FInput.read((char*) &Header, sizeof(SegYHeader));

      if (FInput.fail())
      {
	  
	  return -1;
      }

      ConvertSegYHeaderLE(&Header);

      float scalco = 1.0;
      if (Header.scalco < 0)
	  scalco = 1.0/(-Header.scalco);
      if (Header.scalco > 0)
	  scalco = Header.scalco;

      float scalel = 1.0;
      if ( Header.scalel < 0 )
        scalel = -1.0/Header.scalel;
      if ( Header.scalel > 0 )
        scalel = Header.scalel;

      CurrentHeader.Nt = Header.ns;
      CurrentHeader.dt = Header.dt / 1000.0 / 1000.0;
      CurrentHeader.T0 = Header.delrt / 1000.0;

      CurrentHeader.sx = Header.sx * scalco;
      CurrentHeader.sy = Header.sy * scalco;
      CurrentHeader.gx = Header.gx * scalco;
      CurrentHeader.gy = Header.gy * scalco;
      CurrentHeader.selev = Header.selev * scalel;
      CurrentHeader.gelev = Header.gelev * scalel;

      CurrentHeader.CDP = Header.cdp;
      CurrentHeader.CDPx = (CurrentHeader.sx+CurrentHeader.gx)/2.0;
      CurrentHeader.CDPy = (CurrentHeader.sy+CurrentHeader.gy)/2.0;
      CurrentHeader.Offx = CurrentHeader.gx-CurrentHeader.sx;
      CurrentHeader.Offy = CurrentHeader.gy-CurrentHeader.sy;
      break;
    }
    default:
      
      return -1;
      break;
  };
  return 0;
}


int TraceFileHandler::AppointCurrentHeader(SegYHeader Header)
{
  float scalco = 1.0;
  if (Header.scalco < 0)  scalco = 1.0/(-Header.scalco);
  if (Header.scalco > 0)  scalco = Header.scalco;

  float scalel = 1.0;
  if ( Header.scalel < 0 )  scalel = -1.0/Header.scalel;
  if ( Header.scalel > 0 )  scalel = Header.scalel;

  CurrentHeader.Nt = Header.ns;
  CurrentHeader.dt = Header.dt / 1000.0 / 1000.0;
  CurrentHeader.T0 = Header.delrt / 1000.0;

  CurrentHeader.sx = Header.sx * scalco;
  CurrentHeader.sy = Header.sy * scalco;
  CurrentHeader.gx = Header.gx * scalco;
  CurrentHeader.gy = Header.gy * scalco;
  CurrentHeader.selev = Header.selev * scalel;
  CurrentHeader.gelev = Header.gelev * scalel;

  CurrentHeader.CDP = Header.cdp;
  CurrentHeader.CDPx = (CurrentHeader.sx+CurrentHeader.gx)/2.0;
  CurrentHeader.CDPy = (CurrentHeader.sy+CurrentHeader.gy)/2.0;
  CurrentHeader.Offx = CurrentHeader.gx-CurrentHeader.sx;
  CurrentHeader.Offy = CurrentHeader.gy-CurrentHeader.sy;

  return 0;
}

int TraceFileHandler::ReadHeaders(briefheadercoord* HeaderMem, const int Ntr)
{
    char buffer[100*(sizeof(SegYHeader) + CurrentHeader.Nt*sizeof(float))];

    int itrace;
    for ( itrace = 0; itrace < Ntr - 100; itrace+=100)
    {
	FInput.read(buffer, 100*(sizeof(SegYHeader) + CurrentHeader.Nt*sizeof(float)));
	if (FInput.fail())
	{
	    
	    return -1;
	}

	SegYHeader* Header;
	for (int ibuf = 0; ibuf < 100; ibuf++)
	{
	    Header = (SegYHeader*) &buffer[ibuf*(sizeof(SegYHeader) + CurrentHeader.Nt*sizeof(float))];

	    switch(F_MODE)
	    {
		case SEGY_BIGENDIAN:
		{
		    ConvertSegYHeaderBE(Header);
		}
		break;
		
		case SEGY_BIGENDIAN_SXFLOAT:
		{
		    ConvertSegYHeaderBEsxfloat(Header);
		}
		break;
		case SU_LITENDIAN:
		{
		    ConvertSegYHeaderLE(Header);
		    break;
		}
		default:
		    
		    return -1;
		    break;
	    };

	    float scalco = 1.0;
	    if (Header->scalco < 0)
	    {
		scalco = 1.0/(-Header->scalco);
	    }
	    if (Header->scalco > 0)
	    {
		scalco = Header->scalco;
	    }
      
	    HeaderMem[itrace + ibuf].sx = Header->sx * scalco;
	    HeaderMem[itrace + ibuf].sy = Header->sy * scalco;
	    HeaderMem[itrace + ibuf].gx = Header->gx * scalco;
	    HeaderMem[itrace + ibuf].gy = Header->gy * scalco;
	}
    }


    for ( ; itrace < Ntr; itrace++)
    {
	FInput.read(buffer, sizeof(SegYHeader) + CurrentHeader.Nt*sizeof(float));
	if (FInput.fail())
	{
	    
	    return -1;
	}

	SegYHeader* Header;
	Header = (SegYHeader*) buffer;

	switch(F_MODE)
	{
	    case SEGY_BIGENDIAN:
	    {
		ConvertSegYHeaderBE(Header);
	    }
	    break;
	    
	    case SEGY_BIGENDIAN_SXFLOAT:
	    {
		ConvertSegYHeaderBEsxfloat(Header);
	    }
	    break;
	    case SU_LITENDIAN:
	    {
		ConvertSegYHeaderLE(Header);
		break;
	    }
	    default:
		
		return -1;
		break;
	};
	

	float scalco = 1.0;
	if (Header->scalco < 0)
	{
	    scalco = 1.0/(-Header->scalco);
	}
	if (Header->scalco > 0)
	{
	    scalco = Header->scalco;
	}
      
	
	HeaderMem[itrace].sx = Header->sx * scalco;
	HeaderMem[itrace].sy = Header->sy * scalco;
	HeaderMem[itrace].gx = Header->gx * scalco;
	HeaderMem[itrace].gy = Header->gy * scalco;
    }

    return 0;
}



void TraceFileHandler::ConvertSegYHeaderBE(SegYHeader* Header)
{
    if (endianess != BIGENDIAN)
    {
	swap_bytes((void*)&Header->sx, 1, sizeof(int));
	swap_bytes((void*)&Header->sy, 1, sizeof(int));
	swap_bytes((void*)&Header->gx, 1, sizeof(int));
	swap_bytes((void*)&Header->gy, 1, sizeof(int));
	swap_bytes((void*)&Header->cdp, 1, sizeof(int));
	swap_bytes((void*)&Header->scalco, 1, sizeof(short));
	swap_bytes((void*)&Header->selev, 1, sizeof(int));
	swap_bytes((void*)&Header->gelev, 1, sizeof(int));
	swap_bytes((void*)&Header->scalel, 1, sizeof(short));
	swap_bytes((void*)&Header->ns, 1, sizeof(short));
	swap_bytes((void*)&Header->dt, 1, sizeof(short));
	swap_bytes((void*)&Header->delrt, 1, sizeof(short));
    }
}
void TraceFileHandler::ConvertSegYHeaderLE(SegYHeader* Header)
{
    if (endianess != LITENDIAN)
    {
	swap_bytes((void*)&Header->sx, 1, sizeof(int));
	swap_bytes((void*)&Header->sy, 1, sizeof(int));
	swap_bytes((void*)&Header->gx, 1, sizeof(int));
	swap_bytes((void*)&Header->gy, 1, sizeof(int));
	swap_bytes((void*)&Header->cdp, 1, sizeof(int));
	swap_bytes((void*)&Header->scalco, 1, sizeof(short));
	swap_bytes((void*)&Header->selev, 1, sizeof(int));
	swap_bytes((void*)&Header->gelev, 1, sizeof(int));
	swap_bytes((void*)&Header->scalel, 1, sizeof(short));
	swap_bytes((void*)&Header->ns, 1, sizeof(short));
	swap_bytes((void*)&Header->dt, 1, sizeof(short));
	swap_bytes((void*)&Header->delrt, 1, sizeof(short));
    }
}
void TraceFileHandler::ConvertSegYHeaderBEsxfloat(SegYHeader* Header)
{
    if (endianess != BIGENDIAN)
    {
	int sx = Header->sx;
	ibm2float((float*) &(sx), 1, endianess);
	Header->sx = static_cast<int> (*((float*) (&sx)));
	
	int sy = Header->sy;
	ibm2float((float*) &(sy), 1, endianess);
	Header->sy = static_cast<int> (*((float*) (&sy)));
	
	int gx = Header->gx;
	ibm2float((float*) &(gx), 1, endianess);
	Header->gx = static_cast<int> (*((float*) (&gx)));
	
	int gy = Header->gy;
	ibm2float((float*) &(gy), 1, endianess);
	Header->gy = static_cast<int> (*((float*) (&gy)));
	
	swap_bytes((void*)&Header->cdp, 1, sizeof(int));
	swap_bytes((void*)&Header->scalco, 1, sizeof(short));
	swap_bytes((void*)&Header->selev, 1, sizeof(int));
	swap_bytes((void*)&Header->gelev, 1, sizeof(int));
	swap_bytes((void*)&Header->scalel, 1, sizeof(short));
	swap_bytes((void*)&Header->ns, 1, sizeof(short));
	swap_bytes((void*)&Header->dt, 1, sizeof(short));
	swap_bytes((void*)&Header->delrt, 1, sizeof(short));
    }
}
