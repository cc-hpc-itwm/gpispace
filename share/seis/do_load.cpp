#include <do_load.hpp>

#include <iostream>
#include <fstream>

#include <stdexcept>

#include "SegYBHeader.h"
#include "SegYEBCHeader.h"
#include "SegYHeader.h"
#include "swap_bytes.h"

// ************************************************************************* //

#ifdef __cplusplus
extern "C"
{
#endif

void do_load ( const std::string & filename
             , const std::string & type
             , const long & part      // ordinal number of this part
             , const long & part_size // size of a full part, bytes
             , const long & size      // size of this part, bytes
             , const long & num       // number of traces in this part
             , void * pos
             )
{
  if (type == "text")
    {
      FILE * inp (fopen (filename.c_str(), "r"));

      if (inp == NULL)
        {
          throw std::runtime_error ("do_load: could not open " + filename);
        }

      fseek (inp, part * part_size, SEEK_SET);

      fread (pos, size, 1, inp);

      fclose (inp);
    }
  else if ((type == "segy") || (type == "su") )
    {
       FILE * inp (fopen (filename.c_str(), "rb"));

       if (inp == NULL)
         {
           throw std::runtime_error ("do_load: could not open " + filename);
         }

       const size_t size_of_fileheader = (type == "segy")?sizeof(SegYBHeader) + sizeof(SegYEBCHeader):0;

      fseek (inp, size_of_fileheader + part * part_size, SEEK_SET);

      fread (pos, size, 1, inp);

      fclose (inp);

      if (type == "segy")
        {
          const int endianess( LITENDIAN );
          const size_t trace_size( size/num );
          const int Nsample = (trace_size - sizeof(SegYHeader)) / sizeof(float);

          for (int inum = 0; inum < num; ++inum)
            {
              SegYHeader * Header = (SegYHeader*) ( (char*)pos + inum * trace_size);
              float * Data = (float*) ( (char*)pos + inum * trace_size + sizeof(SegYHeader));

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
	      ibm2float(Data, Nsample, endianess);
            }
        }
    }
  else
    {
      throw std::runtime_error ("do_load: unknown type " + type);
    }
}

#ifdef __cplusplus
}
#endif
