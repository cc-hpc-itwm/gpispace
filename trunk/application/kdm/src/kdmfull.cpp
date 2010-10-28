#include <we/loader/macros.hpp>
#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <string>
#include <fstream>

// KDM include files
// #include "structures/migrationjob.h"
// #include "filehandler/checkreadmigrationjob.h"
// #include "filehandler/migrationfilehandler.h"
// #include "TraceBunch.hpp"
// #include "MigSubVol.hpp"
// #include "sdpa_migrate.hpp"

// #include "ttvmmemhandler.h"

// #include "sinc_mod.hpp"

using we::loader::get;
using we::loader::put;

// ************************************************************************* //

static void initialize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const std::string & filename (get<std::string> (input, "file_config"));
  const long & memsizeGPI (get<long> (input, "memsizeGPI"));

  MLOG (INFO, "initialize: filename " << filename);
  MLOG (INFO, "initialize: memsizeGPI " << memsizeGPI);

  put (output, "config", "offsets", 3L);
  put (output, "config", "per.offset.volumes", 5L);
  put (output, "config", "per.offset.bunches", 12L);
  put (output, "config", "per.volume.copies", 2L);
  put (output, "config", "per.volume_store.bunch_stores", 2L);

  put (output, "config", "size.store.bunch", 8L);
  put (output, "config", "size.store.volume", 10L);

  put (output, "config", "assign.most", 4L);

  put (output, "config", "loadTT.parallel", 2L);
}

// ************************************************************************* //

static void loadTT (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const long & id (get<long> (input, "id"));
  const long & parallel (get<long> (config, "loadTT.parallel"));

  MLOG (INFO, "loadTT: id " << id << " out of " << parallel);

  put (output, "done", control());
}

// ************************************************************************* //

static void load (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & bunch (get<value::type> (input, "bunch"));

  MLOG (INFO, "load: bunch " << bunch);

  put (output, "bunch", bunch);
}

// ************************************************************************* //

static void initialize_volume (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & volume (get<value::type> (input, "volume"));

  MLOG (INFO, "initialize_volume: volume " << volume);

  put (output, "volume", volume);
}

// ************************************************************************* //

static void process (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & volume (get<value::type> (input, "volume"));

  MLOG (INFO, "process: volume " << volume);

  put (output, "volume", volume);
}

// ************************************************************************* //

static void reduce (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & volume (get<value::type> (input, "volume"));
  const value::type & sum (get<value::type> (input, "sum"));

  MLOG (INFO, "reduce: volume " << volume);
  MLOG (INFO, "reduce: sum " << sum);

  put (output, "sum", sum);
}

// ************************************************************************* //

static void write (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & volume (get<value::type> (input, "volume"));

  MLOG (INFO, "write: volume " << volume);

  put (output, "volume", volume);
}

// ************************************************************************* //

static void finalize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));

  MLOG (INFO, "finalize: config " << config);

  put (output, "trigger", control());
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (kdmfull);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (kdmfull)");

  WE_REGISTER_FUN (initialize);
  WE_REGISTER_FUN (loadTT);
  WE_REGISTER_FUN (load);
  WE_REGISTER_FUN (initialize_volume);
  WE_REGISTER_FUN (process);
  WE_REGISTER_FUN (reduce);
  WE_REGISTER_FUN (write);
  WE_REGISTER_FUN (finalize);
}
WE_MOD_INITIALIZE_END (kdmfull);

WE_MOD_FINALIZE_START (kdmfull);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (kdmfull)");
}
WE_MOD_FINALIZE_END (kdmfull);
