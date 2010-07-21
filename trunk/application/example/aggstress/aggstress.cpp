#include <we/loader/macros.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <string>
#include <fstream>

#include <stdexcept>

static unsigned long call_cnt = 0;

#define MAGIC 4711

static void alloc ( void *
                  , const we::loader::input_t & input
                  , we::loader::output_t & output
                  )
{
  const fvmAllocHandle_t handle (fvmGlobalAlloc (sizeof(unsigned long)));
  const fvmAllocHandle_t scratch (fvmGlobalAlloc (sizeof(unsigned long)));

  if (!handle)
    {
      throw std::runtime_error ("BUMMER: alloc (handle) returned 0");
    }

  if (!scratch)
    {
      throw std::runtime_error ("BUMMER: alloc (scratch) returned 0");
    }

  unsigned long magic (MAGIC);

  memcpy (fvmGetShmemPtr(), &magic, sizeof(magic));

  fvmPutGlobalData (handle, 0, sizeof(unsigned long), 0, scratch);

  value::structured_t config;

  config["handle"] = static_cast<long>(handle);
  config["scratch"] = static_cast<long>(scratch);

  we::loader::put_output (output, "config", config);
}

static void run ( void *
                , const we::loader::input_t & input
                , we::loader::output_t & output
                )
{
  ++call_cnt;

  const value::type & config (input.value ("config"));

  const fvmAllocHandle_t handle
    (value::get_literal_value<long> (value::get_field ("handle", config)));
  const fvmAllocHandle_t scratch
    (value::get_literal_value<long> (value::get_field ("scratch", config)));

  unsigned long magic;

  fvmGetGlobalData (handle, 0, sizeof(unsigned long), 0, scratch);

  memcpy (&magic, fvmGetShmemPtr(), sizeof(unsigned long));

  if (magic != MAGIC)
    {
      std::ostringstream s;

      s << "expected " << MAGIC << " got " << magic;

      throw std::runtime_error ("BUMMER: " + s.str());
    }

  we::loader::put_output (output, "done", control());
}

static void free ( void *
                  , const we::loader::input_t & input
                  , we::loader::output_t & output
                  )
{
  MLOG (INFO, "call_cnt = " << call_cnt);

  const value::type & config (input.value ("config"));

  const fvmAllocHandle_t handle
    (value::get_literal_value<long> (value::get_field ("handle", config)));
  const fvmAllocHandle_t scratch
    (value::get_literal_value<long> (value::get_field ("scratch", config)));

  if (!handle)
    {
      throw std::runtime_error ("STRANGE! BUMMER: free (handle) got 0");
    }
  
  if (!scratch)
    {
      throw std::runtime_error ("STRANGE! BUMMER: free (scratch) got 0");
    }

  fvmGlobalFree (handle);
  fvmGlobalFree (scratch);

  we::loader::put_output (output, "trigger", control());
}

WE_MOD_INITIALIZE_START (aggstress);
{
  WE_REGISTER_FUN (alloc);
  WE_REGISTER_FUN (run);
  WE_REGISTER_FUN (free);
}
WE_MOD_INITIALIZE_END (aggstress);

WE_MOD_FINALIZE_START (aggstress);
{
}
WE_MOD_FINALIZE_END (aggstress);
