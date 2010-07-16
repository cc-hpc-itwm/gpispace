/***************************************************************************
                          seismicfilewriter.cpp  -  description
                             -------------------
    begin                : Thu Feb 23 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

#include "seismicfilewriter.h"

SeismicFileWriter::SeismicFileWriter()
{
    endianess = TestByteOrder();
}

SeismicFileWriter::SeismicFileWriter(const char* Name, FILE_MODE _F_MODE, const short _Nt, const short _dt, int& ierr)
{
  endianess = TestByteOrder();
  ierr = Open(Name, _F_MODE, _Nt, _dt);
}

int SeismicFileWriter::Open(const char* Name, FILE_MODE _F_MODE, const short _Nt, const short _dt)
{
  Nt = _Nt;
  int ierr = 0;
  F_MODE = _F_MODE;

  
  //
  FOutput.open(Name, std::ios::binary);
  if (FOutput.fail())
  {
    
    ierr = -1;
    return ierr;
  }

  bytes_per_trace = sizeof(SegYHeader) + Nt * sizeof(float);

  switch(F_MODE)
  {
      case SEGY_BIGENDIAN:
      {
	  
	  SegYEBCHeader EBCHeader = {};
	  SegYBHeader BHeader;	

	  BHeader.hns = Nt;
	  BHeader.hdt = _dt;
	  if (endianess != BIGENDIAN)
	    {
		swap_bytes((void*)&BHeader.hns, 1, sizeof(short));
		swap_bytes((void*)&BHeader.hdt, 1, sizeof(short));
		swap_bytes((void*)&BHeader.format, 1, sizeof(short));	  
	    }
	  FOutput.write((char*) &EBCHeader, sizeof(SegYEBCHeader));
	  FOutput.write((char*) &BHeader, sizeof(SegYBHeader));
	  if (FOutput.fail())
	  {
	      
	      ierr = -1;
	      return ierr;
	  }
	
	  file_offset = sizeof(SegYEBCHeader) + sizeof(SegYBHeader);

	  break;
      }
      case SU_LITENDIAN:
      {
	  
	
	  file_offset = 0;

	  break;
      }
      default:
      {
	  
	  break;
      }
  }
  return ierr;
}


SeismicFileWriter::~SeismicFileWriter(){
    Close();
}

void SeismicFileWriter::GoTo(long trace_number) {
    FOutput.seekp(file_offset + trace_number*bytes_per_trace);
}


int SeismicFileWriter::Close(){
    FOutput.close();
    if (FOutput.fail())
	return -1;
    return 0;
}
int SeismicFileWriter::Write(const SegYHeader* Header, const float* Mem)
{
  //
  if (Mem == NULL)
  {
      
      return -1;
  }

  switch(F_MODE)
  {
    case SEGY_BIGENDIAN:
    {
	SegYHeader LocalHeader = *Header;

	float* LocalMem = new float[Nt];
	memcpy(LocalMem, Mem, Nt*sizeof(float));

	if (endianess != BIGENDIAN)
	{
	    swap_bytes((void*)&LocalHeader.tracl, 1, sizeof(int));
	    swap_bytes((void*)&LocalHeader.tracr, 1, sizeof(int));
	    swap_bytes((void*)&LocalHeader.tracf, 1, sizeof(int));
	    swap_bytes((void*)&LocalHeader.cdp, 1, sizeof(int));
	    swap_bytes((void*)&LocalHeader.cdpt, 1, sizeof(int));
	    swap_bytes((void*)&LocalHeader.ep, 1, sizeof(int));
	    swap_bytes((void*)&LocalHeader.scalco, 1, sizeof(short));
	    swap_bytes((void*)&LocalHeader.sx, 1, sizeof(int));
	    swap_bytes((void*)&LocalHeader.sy, 1, sizeof(int));
	    swap_bytes((void*)&LocalHeader.gx, 1, sizeof(int));
	    swap_bytes((void*)&LocalHeader.gy, 1, sizeof(int));
	    swap_bytes((void*)&LocalHeader.offset, 1, sizeof(int));
	    swap_bytes((void*)&LocalHeader.selev, 1, sizeof(int));
	    swap_bytes((void*)&LocalHeader.gelev, 1, sizeof(int));
	    swap_bytes((void*)&LocalHeader.scalel, 1, sizeof(short));
	    swap_bytes((void*)&LocalHeader.ns, 1, sizeof(unsigned short));
	    swap_bytes((void*)&LocalHeader.dt, 1, sizeof(unsigned short));
	    swap_bytes((void*)&LocalHeader.trid, 1, sizeof(short));
	    swap_bytes((void*)&LocalHeader.delrt, 1, sizeof(short));
	    swap_bytes((void*)&LocalHeader.gdel, 1, sizeof(int));
	    swap_bytes((void*)&LocalHeader.sdel, 1, sizeof(int));
	}
	float2ibm(LocalMem, Nt, endianess);

#ifdef _use_wavelet_trafo
      swap_bytes(&LocalHeader.sstack_wt_nt, 1, sizeof(int));
      swap_bytes(&LocalHeader.sstack_wt_dt, 1, sizeof(float));
      swap_bytes(&LocalHeader.sstack_wt_t0, 1, sizeof(float));
      swap_bytes(&LocalHeader.sstack_wt_f0, 1, sizeof(float));
      swap_bytes(&LocalHeader.sstack_wt_df, 1, sizeof(float));
      swap_bytes(&LocalHeader.sstack_wt_f1, 1, sizeof(float));
#endif

	FOutput.write((char*) &LocalHeader, sizeof(SegYHeader));
	FOutput.write((char*) LocalMem, Nt*sizeof(float));

	delete[] LocalMem;

	if (FOutput.fail())
	{
	    
	    return -1;
	}
	
	break;
    }  
    case SU_LITENDIAN:
    {
	FOutput.write((char*) Header, sizeof(SegYHeader));
	FOutput.write((char*) Mem, Nt*sizeof(float));
	if (FOutput.fail())
	{
	    
	    return -1;
	}
      break;
    }

    default:
      
      break;
  };
  return 0;
}
