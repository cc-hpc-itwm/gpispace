#ifndef _DO_WRITE_HPP
#define _DO_WRITE_HPP 1

#include <string>

void do_write ( const std::string & filename
              , const std::string & type
              , const long & part      // ordinal number of this part
              , const long & part_size // size of a full part, bytes
              , const long & size      // size of this part, bytes
              , const long & num       // number of traces in this part
              , void * pos
              );

#endif
