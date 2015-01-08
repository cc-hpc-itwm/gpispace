#include <iostream>
#include <string>
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

void determine_size ( const std::string & filename
                    , const std::string & type
                    , long & num       // number of traces in this file
                    , long & size      // size of one trace + header in bytes
                    )
{
  if (type == "text")
    {
      throw std::runtime_error ("determine_size: note implemented for file type 'text' ");
    }
  else if ( (type == "segy") || (type == "su") )
    {
       const int endianess( LITENDIAN );
       FILE * inp (fopen (filename.c_str(), "rb"));

       if (inp == NULL)
         {
           throw std::runtime_error ("do_load: could not open " + filename);
         }

       if (type == "segy")
	   fseek (inp, sizeof(SegYBHeader) + sizeof(SegYEBCHeader), SEEK_SET);

      SegYHeader Header;
      fread ((char*)&Header, sizeof(SegYHeader), 1, inp);
      if ( (type == "segy") && (endianess != BIGENDIAN) )
      {
	  swap_bytes((void*)&(Header.ns), 1, sizeof(unsigned short));
      }
      size = sizeof(SegYHeader) + Header.ns*sizeof(float);

      fseek (inp, 0, SEEK_END);
      const long endpos = ftell (inp);
      if (type == "segy")
	  num = (endpos - sizeof(SegYBHeader) - sizeof(SegYEBCHeader)) / size;
      else
	  num = (endpos ) / size;
      fclose (inp);
    }
  else
    {
      throw std::runtime_error ("determine_size: unknown type " + type);
    }
}

#ifdef __cplusplus
}
#endif
