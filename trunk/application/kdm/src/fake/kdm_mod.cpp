#include <we/loader/macros.hpp>
#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <string>
#include <fstream>

using we::loader::get;
using we::loader::put;

// ************************************************************************* //

static void initialize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const std::string & filename (get<std::string> (input, "config_file"));

  MLOG (INFO, "initialize: filename " << filename);

  put (output, "config", "handle_Job", 0L);
  put (output, "config", "scratch_Job", 0L);
  put (output, "config", "handle_TT", 0L);
  put (output, "config", "NThreads", 4L);

  put (output, "config", "OFFSETS", 2L);
  put (output, "config", "SUBVOLUMES_PER_OFFSET", 3L);
  put (output, "config", "BUNCHES_PER_OFFSET", 5L);
  put (output, "config", "PARALLEL_LOADTT", 8L);

  put (output, "config", "VOLUME_CREDITS", 16L);

  MLOG (INFO, "initialize: config " << get<value::type>(output, "config"));
}

static void loadTT (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const long & id (get<long> (input, "id"));

  MLOG (INFO, "loadTT: id " << id << ", config " << config);

  put (output, "done", control());
}

static void load (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & bunch (get<value::type> (input, "bunch"));

  MLOG (INFO, "load: bunch " << bunch << ", config " << config);

  put (output, "bunch", bunch);
}

static void process (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & bunch (get<value::type> (input, "bunch"));

  MLOG (INFO, "process: bunch " << bunch << ", config " << config);

  put (output, "bunch", bunch);
}

static void write (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & volume (get<value::type> (input, "volume"));

  MLOG (INFO, "write: volume " << volume << ", config " << config);

  put (output, "volume", volume);
}

static void finalize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));

  MLOG (INFO, "finalize: config " << config);

  put (output, "trigger", control());
}

static void init_volume (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & volume (get<value::type> (input, "volume"));

  MLOG (INFO, "init_volume: volume " << volume << ", config " << config);

  put (output, "volume", volume);
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (kdm);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (kdm_fake)");

  WE_REGISTER_FUN (initialize);
  WE_REGISTER_FUN (loadTT);
  WE_REGISTER_FUN (load);
  WE_REGISTER_FUN (process);
  WE_REGISTER_FUN (write);
  WE_REGISTER_FUN (init_volume);
  WE_REGISTER_FUN (finalize);
}
WE_MOD_INITIALIZE_END (kdm);

WE_MOD_FINALIZE_START (kdm);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (kdm)");
}
WE_MOD_FINALIZE_END (kdm);
