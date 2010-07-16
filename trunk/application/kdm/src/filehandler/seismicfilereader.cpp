/***************************************************************************
                          seismicfilereader.cpp  -  description
                             -------------------
    begin                : Thu Feb 23 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

#include "seismicfilereader.h"

SeismicFileReader::SeismicFileReader()
{
    endianess = TestByteOrder();
    stripe = 0;
    offset = 0;

    file_offset = 0;
    bytes_per_trace = 0;
    file_size = 0;
}

SeismicFileReader::SeismicFileReader(const char* Name, FILE_MODE _F_MODE, const int _Nt, int& ierr)
{
    Nt = _Nt;
    stripe = 0;
    offset = 0;
    endianess = TestByteOrder();
  ierr = 0;
  F_MODE = _F_MODE;

  bytes_per_trace = sizeof(SegYHeader) + Nt * sizeof(float);

  
  FInput.open(Name, std::ios::binary | std::ios::in | std::ios::ate);
  if (FInput.fail())
  {
    
    ierr = -1;
    return;
  }
  file_size = FInput.tellg();
  FInput.seekg(0);

  switch(F_MODE)
  {
      case SEGY_BIGENDIAN:
      {
	  
	  SegYEBCHeader EBCHeader;
	  SegYBHeader BHeader;	
	  FInput.read((char*) &EBCHeader, sizeof(SegYEBCHeader));
	  FInput.read((char*) &BHeader, sizeof(SegYBHeader));
	  if (FInput.fail())
	  {
	      
	      ierr = -1;
	      return;
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
  };
}


SeismicFileReader::SeismicFileReader(const char* Name, FILE_MODE _F_MODE, const int _Nt, const int _stripe, const int _offset, int& ierr)
{
    Nt = _Nt;
    stripe = _stripe - 1;
    offset = _offset;
    endianess = TestByteOrder();
  ierr = 0;
  F_MODE = _F_MODE;

  bytes_per_trace = sizeof(SegYHeader) + Nt * sizeof(float);

  
  FInput.open(Name, std::ios::binary | std::ios::in | std::ios::ate);
  if (FInput.fail())
  {
    
    ierr = -1;
    return;
  }
  file_size = FInput.tellg();
  FInput.seekg(0);


  switch(F_MODE)
  {
      case SEGY_BIGENDIAN:
      {
	  
	  SegYEBCHeader EBCHeader;
	  SegYBHeader BHeader;	
	  FInput.read((char*) &EBCHeader, sizeof(SegYEBCHeader));
	  FInput.read((char*) &BHeader, sizeof(SegYBHeader));
	  if (FInput.fail())
	  {
	      
	      ierr = -1;
	      return;
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
  };

  FInput.seekg( offset * (sizeof(SegYHeader) + Nt*sizeof(float)), std::ios::cur );

}


SeismicFileReader::~SeismicFileReader(){
    Close();
}

void SeismicFileReader::Init(const char* FName, FILE_MODE _F_MODE, const int _Nt, int& ierr) {
    Nt = _Nt;
    stripe = 0;
    offset = 0;
    endianess = TestByteOrder();
  ierr = 0;
  F_MODE = _F_MODE;

  bytes_per_trace = sizeof(SegYHeader) + Nt * sizeof(float);

  
  FInput.open(FName, std::ios::binary | std::ios::in | std::ios::ate);
  if (FInput.fail())
  {
    
    ierr = -1;
    return;
  }
  file_size = FInput.tellg();
  FInput.seekg(0);


  switch(F_MODE)
  {
      case SEGY_BIGENDIAN:
      {
	  
	  SegYEBCHeader EBCHeader;
	  SegYBHeader BHeader;	
	  FInput.read((char*) &EBCHeader, sizeof(SegYEBCHeader));
	  FInput.read((char*) &BHeader, sizeof(SegYBHeader));
	  if (FInput.fail())
	  {
	      
	      ierr = -1;
	      return;
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
  };

};

void SeismicFileReader::SetNt(int _Nt) { 
    Nt=_Nt;   
    bytes_per_trace = sizeof(SegYHeader) + Nt * sizeof(float); 
};

void SeismicFileReader::GoTo(long trace_number) {
    FInput.seekg(file_offset + trace_number*bytes_per_trace);
}

std::ifstream::pos_type SeismicFileReader::GetDataSize() { return (file_size-file_offset); };

int SeismicFileReader::Close(){
    FInput.close();
    if (FInput.fail())
	return -1;
    return 0;
}

int SeismicFileReader::Read(SegYHeader* Header, float* Mem)
{
  if (Mem == NULL)
  {
      
      return -1;
  }

  switch(F_MODE)
  {
    case SEGY_BIGENDIAN:
    {
	FInput.read((char*) Header, sizeof(SegYHeader));
	FInput.read((char*) Mem, Nt*sizeof(float));
	
	if (FInput.fail())
	{
	    
	    return -1;
	}
	
	if (endianess != BIGENDIAN)
	{
	    swap_bytes((void*)&Header->tracl, 1, sizeof(int));
	    swap_bytes((void*)&Header->tracr, 1, sizeof(int));
	    swap_bytes((void*)&Header->tracf, 1, sizeof(int));
	    swap_bytes((void*)&Header->cdp, 1, sizeof(int));
	    swap_bytes((void*)&Header->cdpt, 1, sizeof(int));
	    swap_bytes((void*)&Header->ep, 1, sizeof(int));
	    swap_bytes((void*)&Header->scalco, 1, sizeof(short));
	    swap_bytes((void*)&Header->sx, 1, sizeof(int));
	    swap_bytes((void*)&Header->sy, 1, sizeof(int));
	    swap_bytes((void*)&Header->gx, 1, sizeof(int));
	    swap_bytes((void*)&Header->gy, 1, sizeof(int));
	    swap_bytes((void*)&Header->offset, 1, sizeof(int));
	    swap_bytes((void*)&Header->selev, 1, sizeof(int));
	    swap_bytes((void*)&Header->gelev, 1, sizeof(int));
	    swap_bytes((void*)&Header->scalel, 1, sizeof(short));
	    swap_bytes((void*)&Header->ns, 1, sizeof(unsigned short));
	    swap_bytes((void*)&Header->dt, 1, sizeof(unsigned short));
	    swap_bytes((void*)&Header->trid, 1, sizeof(short));
	    swap_bytes((void*)&Header->delrt, 1, sizeof(short));
	    swap_bytes((void*)&Header->gdel, 1, sizeof(int));
	    swap_bytes((void*)&Header->sdel, 1, sizeof(int));
	}
	ibm2float(Mem, Nt, endianess);
	break;
    }  
    case SU_LITENDIAN:
    {
	FInput.read((char*) Header, sizeof(SegYHeader));
	FInput.read((char*) Mem, Nt*sizeof(float));
	if (FInput.fail())
	{
	    
	    return -1;
	}
      break;
    }

    default:
      
      break;
  };

   if (stripe > 0)
       FInput.seekg( stripe * (sizeof(SegYHeader) + Nt*sizeof(float)), std::ios::cur );
  
  return 0;
}

int SeismicFileReader::Read(SegYHeader* Header)
{
  switch(F_MODE)
  {
    case SEGY_BIGENDIAN:
    {
	FInput.read((char*) Header, sizeof(SegYHeader));
	
	if (FInput.fail())
	{
	    
	    return -1;
	}
	
	swap_bytes(&(Header->tracl), 1, sizeof(int));
	swap_bytes(&(Header->tracr), 1, sizeof(int));
	swap_bytes(&(Header->offset), 1, sizeof(int));
	swap_bytes(&(Header->sx), 1, sizeof(int));
	swap_bytes(&(Header->sy), 1, sizeof(int));
	swap_bytes(&(Header->scalco), 1, sizeof(short));
	swap_bytes(&(Header->ns), 1, sizeof(short));
	swap_bytes(&(Header->dt), 1, sizeof(short));
	swap_bytes(&(Header->delrt), 1, sizeof(short));

	swap_bytes(&Header->gx, 1, sizeof(int));
	swap_bytes(&Header->gy, 1, sizeof(int));
	swap_bytes(&Header->cdp, 1, sizeof(int));
	swap_bytes(&Header->selev, 1, sizeof(int));
	swap_bytes(&Header->gelev, 1, sizeof(int));
	swap_bytes(&Header->scalel, 1, sizeof(short));
	swap_bytes(&Header->trid, 1, sizeof(short));

	swap_bytes(&Header->cdpt, 1, sizeof(int));
	swap_bytes(&Header->gdel, 1, sizeof(int));
	swap_bytes(&Header->sdel, 1, sizeof(int));

	break;
    }  
    case SU_LITENDIAN:
    {
	FInput.read((char*) Header, sizeof(SegYHeader));
	if (FInput.fail())
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
