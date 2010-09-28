#include <we/loader/macros.hpp>
#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <string>
#include <fstream>

#include "TraceBunch.hpp"
#include "TraceData.hpp"

using we::loader::get;
using we::loader::put;

// ************************************************************************* //

static void generic_filter ( void * state
                           , const we::loader::input_t & input
                           , we::loader::output_t & output
                           , void (*filter_impl) (void *, const long &)
                           )
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & part_in_store (get<value::type> (input, "in"));
  const long & store (get<long> (part_in_store, "id.store"));

  MLOG (INFO, "generic_filter: store " << store << ", config " << config);

  // communicate from GPI space to fvmGetShmemPtr()

  const fvmAllocHandle_t & handle_data (get<long> (config, "handle.data"));
  const fvmAllocHandle_t & handle_scratch (get<long> (config, "handle.scratch"));
  const long & sizeofBunchBuffer (get<long> (config, "bunchbuffer.size"));

  waitComm (fvmGetGlobalData ( handle_data
                             , store * sizeofBunchBuffer
                             , sizeofBunchBuffer
                             , 0
                             , handle_scratch
                             )
           );

  // call filter_impl

  filter_impl (fvmGetShmemPtr(), get<long> (config, "tune.trace_per_bunch"));

  // communicate back

  waitComm (fvmPutGlobalData ( handle_data
                             , store * sizeofBunchBuffer
                             , sizeofBunchBuffer
                             , 0
                             , handle_scratch
                             )
           );

  put (output, "out", part_in_store);
}

// ************************************************************************* //

// ptr points to num many basis-items (e.g. traces)
static void unblank_impl (void * ptr, const long & num)
{
  char * buf ((char *) ptr);

  // in the unblank case, we know, that an item has 300 bytes
  for (long i (0); i < 300 * num; ++i)
    {
      if (buf[i] == ' ')
        {
          buf[i] = '.';
        }
    }
}

static void unblank ( void * state
                    , const we::loader::input_t & input
                    , we::loader::output_t & output
                    )
{
  generic_filter (state, input, output, unblank_impl);
}


// ************************************************************************* //

WE_MOD_INITIALIZE_START (filter_trace);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (filter_trace)");

  WE_REGISTER_FUN (unblank);
}
WE_MOD_INITIALIZE_END (filter_trace);

WE_MOD_FINALIZE_START (filter_trace);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (filter_trace)");
}
WE_MOD_FINALIZE_END (filter_trace);
