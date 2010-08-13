#include <we/loader/macros.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <fvm-pc/pc.hpp>

using we::loader::get;
using we::loader::put;

/* ************************************************************************* */

static void initialize ( void *
                       , const we::loader::input_t & input
                       , we::loader::output_t & output
                       )
{
  const std::string & filename (get<std::string> (input, "config_file"));

  value::structured_t config;

  std::cout << "initialize: use file " << filename << std::endl;

  std::ifstream file (filename.c_str());

  if (!file)
    throw std::runtime_error ("Lurcks, config file not good");

  while (!file.eof())
  {
    std::string s;
    file >> s;
    long v;
    file >> v;
    if (s.size())
      config[s] = v;
  }

  std::cout << "initialize: got config " << config << std::endl;

  const long & wait (get<long> (config, "SUBVOLUMES_PER_OFFSET"));

  std::cout << "initialize: wait = " << wait << std::endl;

  put (output, "config", config);
  put (output, "wait", literal::type(wait));
  put (output, "trigger", control());

  bitsetofint::type bs; bs.ins (0);

  put (output, "wanted", bs);

  const long & parallel_loadTT (get<long> (config, "PARALLEL_LOADTT"));

  put (output, "parallel_loadTT", literal::type(parallel_loadTT));
}

/* ************************************************************************* */

static void loadTT ( void *
                   , const we::loader::input_t & input
                   , we::loader::output_t & output
                   )
{
  const value::type & config (get<value::type> (input, "config"));
  const long & TT (get<long> (input, "TT"));

  std::cout << "loadTT: got config " << config << std::endl;
  std::cout << "loadTT: got TT " << TT << std::endl;

  put (output, "TT", TT);
}

/* ************************************************************************* */

static void load ( void *
                 , const we::loader::input_t & input
                 , we::loader::output_t & output
                 )
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & bunch (get<value::type> (input, "bunch"));
  const long & empty_store (get<long> (input, "empty_store"));

  std::cout << "load: got config " << config << std::endl;
  std::cout << "load: got bunch " << bunch << std::endl;
  std::cout << "load: got empty store " << empty_store << std::endl;

  value::structured_t loaded_bunch;

  loaded_bunch["bunch"] = bunch;
  loaded_bunch["store"] = empty_store;
  loaded_bunch["seen"] = bitsetofint::type();
  loaded_bunch["wait"] = get<long> (config, "SUBVOLUMES_PER_OFFSET");

  put (output, "loaded_bunch", loaded_bunch);

  std::cout << "load: loaded_bunch " << loaded_bunch << std::endl;
}

/* ************************************************************************* */

static void process ( void *
                    , const we::loader::input_t & input
                    , we::loader::output_t & output
                    )
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & volume (get<value::type> (input, "volume"));

  std::cout << "process: got config " << config << std::endl;
  std::cout << "process: got volume " << volume << std::endl;

  const value::type & buffer0 (get<value::type> (volume, "buffer0"));
  const bool & assigned0 (get<bool>(buffer0, "assigned"));
  const bool & filled0 (get<bool>(buffer0, "filled"));

  const value::type & buffer1 (get<value::type> (volume, "buffer1"));
  const bool & assigned1 (get<bool>(buffer1, "assigned"));
  const bool & filled1 (get<bool>(buffer1, "filled"));

  // die hier implementierte Logik ist noch nicht optimal: es wird
  // einfach nur jeder bufffer geladen, der assigned aber nicht
  // gefüllt ist und die buffer, die geladen sind, werden verarbeitet.

  const long & wait (get<long>(volume, "wait"));

  value::type volume_processed (volume);

  value::field("assigned", value::field("buffer0", volume_processed)) = assigned0 && !filled0;
  value::field("filled", value::field("buffer0", volume_processed)) = assigned0 && !filled0;
  value::field("free", value::field("buffer0", volume_processed)) = assigned0 && !filled0;
  value::field("assigned", value::field("buffer1", volume_processed)) = assigned1 && !filled1;
  value::field("filled", value::field("buffer1", volume_processed)) = assigned1 && !filled1;
  value::field("free", value::field("buffer1", volume_processed)) = assigned1 && !filled1;
  value::field("wait", volume_processed) = wait - ((filled0) ? 1 : 0) - ((filled1) ? 1 : 0);

  put (output, "volume_processed", volume_processed);

  std::cout << "process: volume_processed " << volume_processed << std::endl;
}

/* ************************************************************************* */

static void write ( void *
                  , const we::loader::input_t & input
                  , we::loader::output_t & output
                  )
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & volume (get<value::type> (input, "volume"));

  std::cout << "write: got config " << config << std::endl;
  std::cout << "write: got volume " << volume << std::endl;

  put (output, "volume", volume);
}

/* ************************************************************************* */

static void finalize ( void *
                     , const we::loader::input_t &  input
                     , we::loader::output_t & output
                     )
{
  const value::type & config (get<value::type> (input, "config"));

  std::cout << "finalize: got config " << config << std::endl;

  put (output, "done", control());
}

/* ************************************************************************* */

static void selftest (void *, const we::loader::input_t & , we::loader::output_t & output)
{
  put (output, "result", 0L);
}

/* ************************************************************************* */

WE_MOD_INITIALIZE_START (kdm_complex);
{
  WE_REGISTER_FUN (initialize);
  WE_REGISTER_FUN (loadTT);
  WE_REGISTER_FUN (load);
  WE_REGISTER_FUN (process);
  WE_REGISTER_FUN (write);
  WE_REGISTER_FUN (finalize);
  WE_REGISTER_FUN (selftest);
}
WE_MOD_INITIALIZE_END (kdm_complex);

WE_MOD_FINALIZE_START (kdm_complex);
{
}
WE_MOD_FINALIZE_END (kdm_complex);
