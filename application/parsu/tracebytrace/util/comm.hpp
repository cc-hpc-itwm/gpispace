#ifndef _COMM_HPP
#define _COMM_HPP 1

#include <pnetc/type/tracebytrace_config.hpp>
#include <pnetc/type/tracebytrace_loaded_package.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <fvm-pc/pc.hpp>

#include <print.hpp>

namespace comm
{
  inline void put
  ( const ::pnetc::type::tracebytrace_config::type config
  , const ::pnetc::type::tracebytrace_loaded_package::type package
  , const long shmem_offset = 0
  )
  {
    LOG (INFO, "comm::put " << ::print::loaded_package (package)
        << " offset " << package.slot * config.size.slot.gpi
        << " from shmem_offset " << shmem_offset
        << " (" << package.package.size.package << " bytes)"
        );

    waitComm ( fvmPutGlobalData
               ( static_cast<fvmAllocHandle_t> (config.handle.data)
               , package.slot * config.size.slot.gpi
               , package.package.size.package
               , shmem_offset
               , 0
               )
             );
  }

  inline void get
  ( const ::pnetc::type::tracebytrace_config::type config
  , const ::pnetc::type::tracebytrace_loaded_package::type package
  , const long shmem_offset = 0
  )
  {
    LOG (INFO, "comm::get " << ::print::loaded_package (package)
        << " offset " << package.slot * config.size.slot.gpi
        << " to shmem_offset " << shmem_offset
        << " (" << package.package.size.package << " bytes)"
        );

    waitComm ( fvmGetGlobalData
               ( static_cast<fvmAllocHandle_t> (config.handle.data)
               , package.slot * config.size.slot.gpi
               , package.package.size.package
               , shmem_offset
               , 0
               )
             );
  }
}

#endif
