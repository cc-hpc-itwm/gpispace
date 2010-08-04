#include <we/loader/macros.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <fvm-pc/pc.hpp>

/* ************************************************************************* */

static void initialize ( void *
                       , const we::loader::input_t & input
                       , we::loader::output_t & output
                       )
{
  const std::string & filename
    (we::loader::get<std::string> (input, "config_file"));

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

  const long wait
    (value::get<long>
     (value::get_field ("SUBVOLUMES_PER_OFFSET", config))
    )
    ;

  std::cout << "initialize: wait = " << wait << std::endl;

  we::loader::put_output (output, "config", config);
  we::loader::put_output (output, "wait", literal::type(wait));
  we::loader::put_output (output, "trigger", control());

  bitsetofint::type bs; bs.ins (0);

  we::loader::put_output (output, "wanted", bs);

  const long parallel_loadTT
    (value::get<long>
     (value::get_field ("PARALLEL_LOADTT", config))
    )
    ;

  we::loader::put_output (output, "parallel_loadTT", literal::type(parallel_loadTT));
}

/* ************************************************************************* */

static void loadTT ( void *
                   , const we::loader::input_t &  input
                   , we::loader::output_t & output
                   )
{
  const value::type & config (input.value("config"));
  const long & TT (we::loader::get<long> (input, "TT"));

  std::cout << "loadTT: got config " << config << std::endl;
  std::cout << "loadTT: got TT " << TT << std::endl;

  we::loader::put_output (output, "TT", TT);
}

/* ************************************************************************* */

static void load ( void *
                 , const we::loader::input_t & input
                 , we::loader::output_t & output
                 )
{
  const value::type & config (input.value("config"));
  const value::type & bunch (input.value("bunch"));
  const long & empty_store (we::loader::get<long> (input, "empty_store"));

  std::cout << "load: got config " << config << std::endl;
  std::cout << "load: got bunch " << bunch << std::endl;
  std::cout << "load: got empty store " << empty_store << std::endl;

  value::structured_t loaded_bunch;

  loaded_bunch["bunch"] = bunch;
  loaded_bunch["store"] = empty_store;
  loaded_bunch["seen"] = bitsetofint::type();
  loaded_bunch["wait"] = value::get<long>
    (value::get_field ("SUBVOLUMES_PER_OFFSET", config));

  we::loader::put_output (output, "loaded_bunch", loaded_bunch);

  std::cout << "load: loaded_bunch " << loaded_bunch << std::endl;
}

/* ************************************************************************* */

static void process ( void *
                    , const we::loader::input_t & input
                    , we::loader::output_t & output
                    )
{
  const value::type & config (input.value("config"));
  const value::type & volume (input.value("volume"));

  std::cout << "process: got config " << config << std::endl;
  std::cout << "process: got volume " << volume << std::endl;

  value::type volume_processed (volume);

  const value::type buffer0 (value::get_field ("buffer0", volume));
  const bool assigned0
    (value::get<bool>(value::get_field ("assigned", buffer0)));
  const bool filled0
    (value::get<bool>(value::get_field ("filled", buffer0)));

  const value::type buffer1 (value::get_field ("buffer1", volume));
  const bool assigned1
    (value::get<bool>(value::get_field ("assigned", buffer1)));
  const bool filled1
    (value::get<bool>(value::get_field ("filled", buffer1)));

  // die hier implementierte Logik ist noch nicht optimal: es wird
  // einfach nur jeder bufffer geladen, der assigned aber nicht
  // gefüllt ist und die buffer, die geladen sind, werden verarbeitet.

  const long wait
    (value::get<long>(value::get_field ("wait", volume)));

  value::field("assigned", value::field("buffer0", volume_processed)) = assigned0 && !filled0;
  value::field("filled", value::field("buffer0", volume_processed)) = assigned0 && !filled0;
  value::field("free", value::field("buffer0", volume_processed)) = assigned0 && !filled0;
  value::field("assigned", value::field("buffer1", volume_processed)) = assigned1 && !filled1;
  value::field("filled", value::field("buffer1", volume_processed)) = assigned1 && !filled1;
  value::field("free", value::field("buffer1", volume_processed)) = assigned1 && !filled1;
  value::field("wait", volume_processed) = wait - ((filled0) ? 1 : 0) - ((filled1) ? 1 : 0);

  we::loader::put_output (output, "volume_processed", volume_processed);

  std::cout << "process: volume_processed " << volume_processed << std::endl;
}

/* ************************************************************************* */

static void write ( void *
                  , const we::loader::input_t & input
                  , we::loader::output_t & output
                  )
{
  const value::type & config (input.value("config"));
  const value::type & volume (input.value("volume"));

  std::cout << "write: got config " << config << std::endl;
  std::cout << "write: got volume " << volume << std::endl;

  we::loader::put_output (output, "volume", volume);
}

/* ************************************************************************* */

static void finalize ( void *
                     , const we::loader::input_t &  input
                     , we::loader::output_t & output
                     )
{
  const value::type & config (input.value("config"));

  std::cout << "finalize: got config " << config << std::endl;

  we::loader::put_output (output, "done", control());
}

/* ************************************************************************* */

static void selftest (void *, const we::loader::input_t & , we::loader::output_t & output)
{
  we::loader::put_output (output, "result", 0L);
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
