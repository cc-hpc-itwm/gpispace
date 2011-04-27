#ifndef FREAD_H_
#define FREAD_H_ 1

#include "pc.hpp"

int read_file ( const int iPart
		         , const int nPart
		         , const char * filename
		         , void * buf
		         , const size_t buf_size
		         , const fvmAllocHandle_t hdl
		         );

#endif /* FREAD_H_ */
