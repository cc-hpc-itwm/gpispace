#include <we/loader/macros.hpp>
#include <we/loader/putget.hpp>
#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <string>
#include <fstream>

using we::loader::get;

// ************************************************************************* //

static void initialize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const std::string & filename (get<std::string> (input, "file_config"));

  MLOG (INFO, "initialize: filename " << filename);

  std::ifstream file (filename.c_str());

  if (!file)
    {
      throw std::runtime_error ("BUMMER: file not good");
    }

  while (!file.eof())
    {
      std::string s;
      file >> s;
      long v;
      file >> v;
      if (s.size())
        {
          output.bind ("config", s, v);
        }
    }

  if ( get<long> (output, "config", "size.store.volume")
     < get<long> (output, "config", "per_offset.volumes")
     )
    {
      throw std::runtime_error
        ("need at least as many volume stores as volumes per offset");
    }

  MLOG (INFO, "initialize: config " << get<value::type>(output, "config"));
}

static void loadTT (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const long & id (get<long> (input, "id"));

  MLOG (INFO, "loadTT: id " << id);

  output.bind ("done", we::type::literal::control());
}

static void load (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & bunch (get<value::type> (input, "bunch"));

  MLOG (INFO, "load: bunch " << bunch);

  output.bind ("bunch", bunch);
}

static void process (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & volume (get<value::type> (input, "volume"));
  literal::stack_type stack_bunch_id
    (get<literal::stack_type> (volume, "assigned.bunch.id"));
  literal::stack_type stack_store_id
    (get<literal::stack_type> (volume, "assigned.bunch.store.id"));

  MLOG (INFO, "process: size " << stack_bunch_id.size());

  while (!stack_bunch_id.empty())
    {
      if (stack_store_id.empty())
        {
          throw std::runtime_error ("BUMMER!");
        }

      const long & bid (stack_bunch_id.back());
      const long & store (stack_store_id.back());
      const long & vid (get<long> (volume, "id"));
      const long & oid (get<long> (volume, "offset.id"));

      MLOG ( INFO
           , "process: match volume " << oid << "." << vid
           << " with bunch " << bid << " from store " << store
           );

      stack_bunch_id.pop_back();
      stack_store_id.pop_back();
    }

  MLOG (INFO, "process: volume " << volume);

  output.bind ("volume", volume);
}

static void write (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & volume (get<value::type> (input, "volume"));

  MLOG (INFO, "write: volume " << volume);

  output.bind ("volume", volume);
}

static void finalize (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & config (get<value::type> (input, "config"));

  MLOG (INFO, "finalize: config " << config);

  output.bind ("trigger", we::type::literal::control());
}

static void init_volume (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  const value::type & volume (get<value::type> (input, "volume"));

  MLOG (INFO, "init_volume: volume " << volume);

  output.bind ("volume", volume);
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (kdm_complex);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (kdm_complex)");

  WE_REGISTER_FUN (initialize);
  WE_REGISTER_FUN (loadTT);
  WE_REGISTER_FUN (load);
  WE_REGISTER_FUN (process);
  WE_REGISTER_FUN (write);
  WE_REGISTER_FUN (init_volume);
  WE_REGISTER_FUN (finalize);
}
WE_MOD_INITIALIZE_END (kdm_complex);

WE_MOD_FINALIZE_START (kdm_complex);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (kdm_complex)");
}
WE_MOD_FINALIZE_END (kdm_complex);
