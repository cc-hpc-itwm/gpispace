#ifndef _DO_LOAD_HPP
#define _DO_LOAD_HPP 1

#include <string>

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
             );

#ifdef __cplusplus
}
#endif

#endif
