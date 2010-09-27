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

static void work ( void * state
                 , const we::loader::input_t & input
                 , we::loader::output_t & output
                 )
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & part_in_store (get<value::type> (input, "in"));
  const long & part (get<long> (part_in_store, "id.part"));
  const long & store (get<long> (part_in_store, "id.store"));

  MLOG (INFO, "work: part " << part << ", store " << store << ", config " << config);

  // communicate from GPI space to fvmGetShmemPtr()

  // call work_impl

  // save to disk from fvmGetShmemPtr()

  put (output, "out", part_in_store);
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (filter_trace);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (filter_trace)");

  WE_REGISTER_FUN (work);
}
WE_MOD_INITIALIZE_END (filter_trace);

WE_MOD_FINALIZE_START (filter_trace);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (filter_trace)");
}
WE_MOD_FINALIZE_END (filter_trace);
