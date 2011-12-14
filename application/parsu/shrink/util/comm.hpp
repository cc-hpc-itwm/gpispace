#ifndef _COMM_HPP
#define _COMM_HPP 1

#include <pnetc/type/shrink_config.hpp>
#include <pnetc/type/shrink_loaded_package.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <fvm-pc/pc.hpp>

#include <print.hpp>

namespace comm
{
  inline void put
  ( const ::pnetc::type::shrink_config::shrink_config config
  , const ::pnetc::type::shrink_loaded_package::shrink_loaded_package package
  , const long shmem_offset = 0
  )
  {
    const long size (package.package.num.trace * package.package.size.trace);

    LOG (INFO, "comm::put " << ::print::loaded_package (package)
        << " offset " << package.slot * config.size.bunch
        << " to shmem_offset " << shmem_offset
        << " (" << size << " bytes)"
        );

    waitComm ( fvmPutGlobalData
               ( static_cast<fvmAllocHandle_t> (config.handle.data)
               , package.slot * config.size.bunch
               , size
               , shmem_offset
               , static_cast<fvmAllocHandle_t> (config.handle.scratch)
               )
             );
  }

  inline void get
  ( const ::pnetc::type::shrink_config::shrink_config config
  , const ::pnetc::type::shrink_loaded_package::shrink_loaded_package package
  , const long shmem_offset = 0
  )
  {
    const long size (package.package.num.trace * package.package.size.trace);

    LOG (INFO, "comm::get " << ::print::loaded_package (package)
        << " offset " << package.slot * config.size.bunch
        << " from shmem_offset " << shmem_offset
        << " (" << size << " bytes)"
        );

    waitComm ( fvmGetGlobalData
               ( static_cast<fvmAllocHandle_t> (config.handle.data)
               , package.slot * config.size.bunch
               , size
               , shmem_offset
               , static_cast<fvmAllocHandle_t> (config.handle.scratch)
               )
             );
  }
}

#endif
