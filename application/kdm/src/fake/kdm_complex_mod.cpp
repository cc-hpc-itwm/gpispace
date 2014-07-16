#include <we/loader/macros.hpp>

#include <fhglog/LogMacros.hpp>
#include <fvm-pc/pc.hpp>

#include <iostream>
#include <string>
#include <fstream>

#include <we/type/value/shpw.hpp>
#include <we/type/value/peek.hpp>

namespace
{
  template<typename R>
    R peek (const pnet::type::value::value_type& x, const std::string& key)
  {
    return boost::get<R> (*pnet::type::value::peek (key, x));
  }
}

// ************************************************************************* //

static void initialize (void *, const expr::eval::context & input, expr::eval::context & output)
{
  const std::string& filename
    (boost::get<const std::string&> (input.value ("file_config")));

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
          output.bind_and_discard_ref ("config", s, v);
        }
    }

  if ( peek<long> (output.value ("config"), "size.store.volume")
     < peek<long> (output.value ("config"), "per_offset.volumes")
     )
    {
      throw std::runtime_error
        ("need at least as many volume stores as volumes per offset");
    }

  MLOG (INFO, "initialize: config " << pnet::type::value::show (output.value ("config")));
}

static void loadTT (void *, const expr::eval::context & input, expr::eval::context & output)
{
  const long& id (boost::get<const long&> (input.value ("id")));

  MLOG (INFO, "loadTT: id " << id);

  output.bind_and_discard_ref ("done", we::type::literal::control());
}

static void load (void *, const expr::eval::context & input, expr::eval::context & output)
{
  const pnet::type::value::value_type& bunch (input.value ("bunch"));

  MLOG (INFO, "load: bunch " << pnet::type::value::show (bunch));

  output.bind_and_discard_ref ("bunch", bunch);
}

static void process (void *, const expr::eval::context & input, expr::eval::context & output)
{
  const pnet::type::value::value_type& volume (input.value ("volume"));
  literal::stack_type stack_bunch_id
    (peek<literal::stack_type> (volume, "assigned.bunch.id"));
  literal::stack_type stack_store_id
    (peek<literal::stack_type> (volume, "assigned.bunch.store.id"));

  MLOG (INFO, "process: size " << stack_bunch_id.size());

  while (!stack_bunch_id.empty())
    {
      if (stack_store_id.empty())
        {
          throw std::runtime_error ("BUMMER!");
        }

      const long & bid (stack_bunch_id.back());
      const long & store (stack_store_id.back());
      const long & vid (peek<long> (volume, "id"));
      const long & oid (peek<long> (volume, "offset.id"));

      MLOG ( INFO
           , "process: match volume " << oid << "." << vid
           << " with bunch " << bid << " from store " << store
           );

      stack_bunch_id.pop_back();
      stack_store_id.pop_back();
    }

  MLOG (INFO, "process: volume " << pnet::type::value::show (volume));

  output.bind_and_discard_ref ("volume", volume);
}

static void write (void *, const expr::eval::context & input, expr::eval::context & output)
{
  const pnet::type::value::value_type& volume (input.value ("volume"));

  MLOG (INFO, "write: volume " << pnet::type::value::show (volume));

  output.bind_and_discard_ref ("volume", volume);
}

static void finalize (void *, const expr::eval::context & input, expr::eval::context & output)
{
  const pnet::type::value::value_type& config (input.value ("config"));

  MLOG (INFO, "finalize: config " << pnet::type::value::show (config));

  output.bind_and_discard_ref ("trigger", we::type::literal::control());
}

static void init_volume (void *, const expr::eval::context & input, expr::eval::context & output)
{
  const pnet::type::value::value_type& volume (input.value ("volume"));

  MLOG (INFO, "init_volume: volume " << pnet::type::value::show (volume));

  output.bind_and_discard_ref ("volume", volume);
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
