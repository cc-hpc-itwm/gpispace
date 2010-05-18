#include <we/loader/macros.hpp>

#include <iostream>
#include <string>
#include <fstream>

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

static void initialize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const std::string & filename
    (we::loader::get_input<std::string> (input, "config_file"));

  long wait (0);
  const value::type & config (kdm_initialize (filename, wait));

  we::loader::put_output (output, "config", config);
  we::loader::put_output (output, "wait", literal::type(wait));
  we::loader::put_output (output, "trigger", control());
}

WE_MOD_INITIALIZE_START (kdm);
{
  WE_REGISTER_FUN (initialize);
}
WE_MOD_INITIALIZE_END (kdm);

WE_MOD_FINALIZE_START (kdm);
{
}
WE_MOD_FINALIZE_END (kdm);
