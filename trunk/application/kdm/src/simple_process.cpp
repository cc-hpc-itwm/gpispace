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

      if (s.size())
        {
          if (  s == "output.file" || s == "output.typ"
             || s == "input.file"  || s == "input.typ"
             )
            {
              std::string v;
              file >> v;

              MLOG (INFO, "init: read " << s << " " << v);

              put (output, "config", s, v);
            }
          else
            {
              long v;
              file >> v;

              MLOG (INFO, "init: read " << s << " " << v);

              put (output, "config", s, v);
            }
        }
    }

  const long & trace_per_bunch (get<long> (output, "config", "tune.trace_per_bunch"));
  const long & trace_num (get<long> (output, "config", "trace_detect.number"));
  const long & trace_size_in_bytes (get<long> (output, "config", "trace_detect.size_in_bytes"));
  const long & memsize (get<long> (output, "config", "tune.memsize"));

  const long sizeofBunchBuffer (trace_per_bunch * trace_size_in_bytes);

  const long node_count (fvmGetNodeCount());

  long num_store_per_node (0);

  while ((1 + num_store_per_node) * node_count * sizeofBunchBuffer < memsize)
    {
      num_store_per_node += 1;
    }

  const fvmAllocHandle_t handle_data
    (fvmGlobalAlloc (num_store_per_node * sizeofBunchBuffer));
  if (handle_data == 0)
    {
      throw std::runtime_error ("BUMMER! handle_data == 0");
    }
  const fvmAllocHandle_t handle_scratch
    (fvmGlobalAlloc (1 * sizeofBunchBuffer));
  if (handle_scratch == 0)
    {
      throw std::runtime_error ("BUMMER! handle_scratch == 0");
    }

  long num_part (trace_num / trace_per_bunch);

  if (trace_num % trace_per_bunch != 0)
    {
      num_part += 1;
    }

  // allocate memory in GPI space
  // allocate scratch handle

  put (output, "config", "num.store", num_store_per_node * node_count);
  put (output, "config", "num.part", num_part);
  put (output, "config", "num.write_credit", 2L);

  put (output, "config", "handle.data", static_cast<long>(handle_data));
  put (output, "config", "handle.scratch", static_cast<long>(handle_scratch));

  MLOG (INFO, "init: got config " << get<value::type>(output, "config"));
}

// ************************************************************************* //

static void finalize ( void * state
                     , const we::loader::input_t & input
                     , we::loader::output_t & output
                     )
{
  const value::type & config (get<value::type> (input, "config"));

  MLOG (INFO, "finalize: config " << config);

  const fvmAllocHandle_t handle_data (get<long> (config, "handle.data"));
  const fvmAllocHandle_t handle_scratch (get<long> (config, "handle.scratch"));

  fvmGlobalFree (handle_data);
  fvmGlobalFree (handle_scratch);

  put (output, "done", control());
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
  WE_REGISTER_FUN (finalize);
}
WE_MOD_INITIALIZE_END (simple_process);

WE_MOD_FINALIZE_START (simple_process);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (simple_process)");
}
WE_MOD_FINALIZE_END (simple_process);
