#include <we/loader/macros.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <fvm-pc/pc.hpp>

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

  wait = value::get_literal_value<long> (value::get_field ("OFFSETS", config))
    * value::get_literal_value<long> (value::get_field ("SUBVOLUMES_PER_OFFSET", config))
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
  const std::string & filename
    (we::loader::get<std::string> (input, "config_file"));

  long wait (0);
  const value::type & config (kdm_initialize (filename, wait));

  we::loader::put_output (output, "config", config);
  we::loader::put_output (output, "wait", literal::type(wait));
  we::loader::put_output (output, "trigger", control());
}

static void loadTT (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & v (input.value("config"));
  kdm_loadTT (v);
  we::loader::put_output (output, "trigger", control());
}

static void load (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (input.value("config"));
  const value::type & bunch (input.value("bunch"));
  kdm_load (config, bunch);
  we::loader::put_output (output, "bunch", bunch);
}

static void process (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (input.value("config"));
  const value::type & bunch (input.value("bunch"));
  long wait (we::loader::get<long>(input, "wait"));
  kdm_process (config, bunch, wait);
  we::loader::put_output (output, "wait", wait);
  we::loader::put_output (output, "trigger", control());
}

static void write (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (input.value("config"));
  const value::type & volume (input.value("volume"));
  kdm_write (config, volume);
  we::loader::put_output (output, "done", control());
}

static void finalize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & v (input.value("config"));
  kdm_finalize (v);
  we::loader::put_output (output, "trigger", control());
}

static void init_volume (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (input.value("config"));
  const value::type & volume (input.value("volume"));
  kdm_init_volume (config, volume);
  we::loader::put_output (output, "volume", volume);
}

static void selftest (void *, const we::loader::input_t & , we::loader::output_t & output)
{
  //  std::cerr << "rank := " << fvmGetRank() << std::endl;
  we::loader::put_output (output, "result", 0L);
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
