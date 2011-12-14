#include <we/loader/macros.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <fvm-pc/pc.hpp>

using we::loader::get;
using we::loader::put;

static value::type kdm_initialize (const std::string & filename, long & wait)
{
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

  wait = get<long> (config, "OFFSETS")
       * get<long> (config, "SUBVOLUMES_PER_OFFSET")
    ;

  std::cout << "initialize: wait = " << wait << std::endl;

  return config;
}

static void kdm_loadTT (const value::type & v)
{
  std::cout << "loadTT: got config " << v << std::endl;
}

static void kdm_finalize (const value::type & v)
{
  std::cout << "finalize: got config " << v << std::endl;
}

static void kdm_load (const value::type & config, const value::type & bunch)
{
  std::cout << "load: got config " << config << std::endl;
  std::cout << "load: got bunch " << bunch << std::endl;
}
static void kdm_write (const value::type & config, const value::type & volume)
{
  std::cout << "write: got config " << config << std::endl;
  std::cout << "write: got volume " << volume << std::endl;
}

static void kdm_process ( const value::type & config
                        , const value::type & bunch
                        , long & wait
                        )
{
  std::cout << "process: got config " << config << std::endl;
  std::cout << "process: got bunch " << bunch << std::endl;
  std::cout << "process: got wait " << wait << std::endl;

  --wait;
}

static void kdm_init_volume ( const value::type & config
                            , const value::type & volume
                            )
{
  std::cout << "init_volume: got config " << config << std::endl;
  std::cout << "init_volume: got volume " << volume << std::endl;
}

// wrapper functions

static void initialize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const std::string & filename (get<std::string> (input, "config_file"));

  long wait (0);
  const value::type & config (kdm_initialize (filename, wait));

  put (output, "config", config);
  put (output, "wait", literal::type(wait));
  put (output, "trigger", control());
}

static void loadTT (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & v (get<value::type> (input, "config"));
  kdm_loadTT (v);
  put (output, "trigger", control());
}

static void load (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & bunch (get<value::type> (input, "bunch"));
  kdm_load (config, bunch);
  put (output, "bunch", bunch);
}

static void process (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & bunch (get<value::type> (input, "bunch"));
  long wait (get<long>(input, "wait"));
  kdm_process (config, bunch, wait);
  put (output, "wait", wait);
}

static void write (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & volume (get<value::type> (input, "volume"));
  kdm_write (config, volume);
  put (output, "done", control());
}

static void finalize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & v (get<value::type> (input, "config"));
  kdm_finalize (v);
  put (output, "trigger", control());
}

static void init_volume (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));
  const value::type & volume (get<value::type> (input, "volume"));
  kdm_init_volume (config, volume);
  put (output, "volume", volume);
}

static void selftest (void *, const we::loader::input_t & , we::loader::output_t & output)
{
  std::cerr << "rank := " << fvmGetRank() << std::endl;
  put (output, "result", 0L);
}

WE_MOD_INITIALIZE_START (kdm);
{
  WE_REGISTER_FUN (initialize);
  WE_REGISTER_FUN (loadTT);
  WE_REGISTER_FUN (load);
  WE_REGISTER_FUN (process);
  WE_REGISTER_FUN (write);
  WE_REGISTER_FUN (init_volume);
  WE_REGISTER_FUN (finalize);
  WE_REGISTER_FUN (selftest);
}
WE_MOD_INITIALIZE_END (kdm);

WE_MOD_FINALIZE_START (kdm);
{
}
WE_MOD_FINALIZE_END (kdm);
