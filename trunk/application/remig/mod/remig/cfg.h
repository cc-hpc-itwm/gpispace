/***************************************
     cfg.h - config file, 
        the struct to be initilized  in the sdpa- init time
        and then to be used as read-only


****************************************/


#ifndef CFG_H
#define CFG_H

#include "reGlbStructs.h"

extern "C" {
   extern void c_read_config(const std::string &config_file, cfg_t *);
}

#endif
