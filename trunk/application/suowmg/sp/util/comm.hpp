#ifndef _COMM_HPP
#define _COMM_HPP 1

#include <pnetc/type/config.hpp>
#include <pnetc/type/assigned_package.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <fvm-pc/pc.hpp>

#include "print.hpp"
#include "util.hpp"

namespace comm
{
  inline void put
  ( const ::pnetc::type::config::config & config
  , const ::pnetc::type::output::output & output
  , long shmem_offset = 0
  )
  {
    LOG ( TRACE
        , "comm::put " << ::print::output (output)
        << " from shmem_offset " << shmem_offset
        << " (" << config.size.output_per_shot << " bytes)"
        );

    fvmAllocHandle_t scratch (fvmLocalAlloc(config.size.output_per_shot));

    try
      {
        waitComm ( fvmPutGlobalData
                   ( static_cast<fvmAllocHandle_t> (config.handle.output.data)
                   , output.slot * config.size.output_per_shot
                   , config.size.output_per_shot
                   , shmem_offset
                   , scratch
                   )
                 );
      }
    catch (const std::exception & e)
      {
        LOG (ERROR, "PUT FAILED: " << e.what());

        if (scratch) fvmLocalFree(scratch);

        throw;
      }

    fvmLocalFree (scratch);
  }

  inline void get
  ( const ::pnetc::type::config::config & config
  , const ::pnetc::type::output::output & output
  , long shmem_offset = 0
  )
  {
    LOG ( TRACE
        , "comm::get " << ::print::output (output)
        << " to shmem_offset " << shmem_offset
        << " (" << config.size.output_per_shot << " bytes)"
        );

    fvmAllocHandle_t scratch (fvmLocalAlloc(config.size.output_per_shot));

    try
      {
        waitComm ( fvmGetGlobalData
                   ( static_cast<fvmAllocHandle_t> (config.handle.output.data)
                   , output.slot * config.size.output_per_shot
                   , config.size.output_per_shot
                   , shmem_offset
                   , scratch
                   )
                 );
      }
    catch (const std::exception & e)
      {
        LOG (ERROR, "GET FAILED: " << e.what());

        if (scratch) fvmLocalFree(scratch);

        throw;
      }

    fvmLocalFree (scratch);
  }

  inline void put
  ( const ::pnetc::type::config::config & config
  , const ::pnetc::type::assigned_package::assigned_package & package
  , long shmem_offset = 0
  )
  {
    LOG ( TRACE
        , "comm::put " << ::print::assigned_package (package)
        << " from shmem_offset " << shmem_offset
        << " (" << ::util::size (package) * config.size.trace << " bytes)"
        );

    fvmAllocHandle_t scratch = fvmLocalAlloc(config.size.bunch);

    for (::util::interval_iterator i (package); i.has_more(); ++i)
      {
        try
          {
            LOG ( TRACE
                , "comm::put to " << ::print::interval (*i)
                << " from shmem_offset " << shmem_offset
                << " (" << (*i).size << " bytes)"
                );

            waitComm ( fvmPutGlobalData
                       ( static_cast<fvmAllocHandle_t> (config.handle.input.data)
                       , (*i).offset
                       , (*i).size
                       , shmem_offset
                       , scratch
                       )
                     );

            shmem_offset += (*i).size;
          }
        catch (const std::exception & e)
          {
            LOG (ERROR, "PUT FAILED: " << e.what());

            if (scratch) fvmLocalFree(scratch);

            throw;
          }
      }

    fvmLocalFree(scratch);
  }

  inline void get
  ( const ::pnetc::type::config::config & config
  , const ::pnetc::type::assigned_package::assigned_package & package
  , long shmem_offset = 0
  )
  {
    LOG ( TRACE
        , "comm::get " << ::print::assigned_package (package)
        << " to shmem_offset " << shmem_offset
        << " (" << ::util::size (package) * config.size.trace << " bytes)"
        );

    fvmAllocHandle_t scratch = fvmLocalAlloc(config.size.bunch);

    for (::util::interval_iterator i (package); i.has_more(); ++i)
      {
        try
          {
            LOG ( TRACE
                , "comm::get from " << ::print::interval (*i)
                << " to shmem_offset " << shmem_offset
                << " (" << (*i).size << " bytes)"
                );

            waitComm ( fvmGetGlobalData
                       ( static_cast<fvmAllocHandle_t> (config.handle.input.data)
                       , (*i).offset
                       , (*i).size
                       , shmem_offset
                       , static_cast<fvmAllocHandle_t> (config.handle.input.scratch)
                       )
                     );

            shmem_offset += (*i).size;
          }
        catch (const std::exception & e)
          {
            LOG (ERROR, "PUT FAILED: " << e.what());

            if (scratch) fvmLocalFree(scratch);

            throw;
          }
      }

    fvmLocalFree(scratch);
  }
}

#endif
