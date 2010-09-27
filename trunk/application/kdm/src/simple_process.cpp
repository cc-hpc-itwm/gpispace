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

static void init ( void * state
		 , const we::loader::input_t & input
		 , we::loader::output_t & output
		 )
{
  const std::string & filename (get<std::string> (input, "desc"));

  MLOG (INFO, "init: got filename " << filename);

  std::ifstream file (filename.c_str());

  if (!file)
    {
      throw std::runtime_error ("BUMMER: file not good");
    }

  MLOG (INFO, "init: read from " << filename);

  while (!file.eof())
    {
      std::string s;
      file >> s;
      long v;
      file >> v;

      MLOG (INFO, "init: read " << s << " " << v);

      if (s.size())
        {
          put (output, "config", s, v);
        }
    }

  const long & trace_per_bunch (get<long> (output, "config", "trace.per_bunch"));
  const long & trace_nt (get<long> (output, "config", "trace.Nt"));
  const long & trace_num (get<long> (output, "config", "trace.number"));
  const long & memsize (get<long> (output, "config", "memsize"));

  const long sizeofTraceBuffer (5 * trace_nt * sizeof(float)
                               + sizeof (int) + 7 * sizeof (float)
                               );
  const long sizeofBunchBuffer (trace_per_bunch * sizeofTraceBuffer);

  const long num_store (memsize / sizeofBunchBuffer);
  long num_part (trace_num / trace_per_bunch);

  if (trace_num % trace_per_bunch != 0)
    {
      num_part += 1;
    }

  // allocate memory in GPI space
  // allocate scratch handle

  put (output, "config", "handle.data", 42L);
  put (output, "config", "handle.scratch", 23L);

  put (output, "config", "num.store", 42L);
  put (output, "config", "num.part", 23L);

  put (output, "config", "num.write_credit", 2L);

  put (output, "config", "file.output", std::string("outputfilename"));

  MLOG (INFO, "got: give config " << get<value::type>(output, "config"));
}

// ************************************************************************* //

static void load ( void * state
		 , const we::loader::input_t & input
		 , we::loader::output_t & output
		 )
{
  const long & part (get<long> (input, "part"));
  const long & store (get<long> (input, "store"));
  const value::type & config (get<value::type> (input, "config"));

  MLOG (INFO, "load: part " << part << ", store " << store << ", config " << config);

  // load data from disk to fvmGetShmemPtr()
  // communicate data to GPI space into handle.data using handle.scratch

  put (output, "part_loaded", "id.part", part);
  put (output, "part_loaded", "id.store", store);
}

// ************************************************************************* //

static void write ( void * state
                  , const we::loader::input_t & input
                  , we::loader::output_t & output
                  )
{
  const value::type & config (get<value::type> (input, "config"));
  const long & part (get<long> (input, "part_in_store", "id.part"));
  const long & store (get<long> (input, "part_in_store", "id.store"));
  const control & credit (get<control> (input, "credit"));

  MLOG (INFO, "write: part " << part << ", store " << store << ", config " << config);

  // communicate from GPI space to fvmGetShmemPtr()
  // save to disk from fvmGetShmemPtr()

  put (output, "part", part);
  put (output, "store", store);
  put (output, "credit", credit);
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (simple_process);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (simple_process)");

  WE_REGISTER_FUN (init);
  WE_REGISTER_FUN (load);
  WE_REGISTER_FUN (write);
}
WE_MOD_INITIALIZE_END (simple_process);

WE_MOD_FINALIZE_START (simple_process);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (simple_process)");
}
WE_MOD_FINALIZE_END (simple_process);
