#ifndef _COMM_HPP
#define _COMM_HPP 1

#include <pnetc/type/stack_config.hpp>
#include <pnetc/type/stack_loaded_package.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <fvm-pc/pc.hpp>

#include <print.hpp>

namespace comm
{
  inline void put
  ( const ::pnetc::type::stack_config::type config
  , const ::pnetc::type::stack_loaded_package::type package
  , const long shmem_offset = 0
  )
  {
    LOG (INFO, "comm::put " << ::print::loaded_package (package)
        << " offset " << package.slot * config.size.bunch
        << " to shmem_offset " << shmem_offset
        << " (" << package.package.size * config.size.trace << " bytes)"
        );

    try
      {
    waitComm ( fvmPutGlobalData
               ( static_cast<fvmAllocHandle_t> (config.handle.data)
               , package.slot * config.size.bunch
               , package.package.size * config.size.trace
               , shmem_offset
               , static_cast<fvmAllocHandle_t> (config.handle.scratch)
               )
             );
      }
    catch (...)
      {
        LOG (INFO, "*** PUT FAILED");

        throw;
      }
  }

  inline void get
  ( const ::pnetc::type::stack_config::type config
  , const ::pnetc::type::stack_loaded_package::type package
  , const long shmem_offset = 0
  )
  {
    LOG (INFO, "comm::get " << ::print::loaded_package (package)
        << " offset " << package.slot * config.size.bunch
        << " from shmem_offset " << shmem_offset
        << " (" << package.package.size * config.size.trace << " bytes)"
        );

    try
      {
    waitComm ( fvmGetGlobalData
               ( static_cast<fvmAllocHandle_t> (config.handle.data)
               , package.slot * config.size.bunch
               , package.package.size * config.size.trace
               , shmem_offset
               , static_cast<fvmAllocHandle_t> (config.handle.scratch)
               )
             );
      }
    catch (...)
      {
        LOG (INFO, "*** GET FAILED");

        throw;
      }
  }
}

#endif
