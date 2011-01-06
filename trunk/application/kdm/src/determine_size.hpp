#ifndef _DETERMINE_SIZE_HPP
#define _DETERMINE_SIZE_HPP 1

#include <string>

void determine_size ( const std::string & filename
                    , const std::string & type
                    , long & num       // number of traces in this file
                    , long & size      // size of one trace + header in bytes
                    );

#endif
