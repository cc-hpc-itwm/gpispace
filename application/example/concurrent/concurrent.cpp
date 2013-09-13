#include <we/loader/macros.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <string>
#include <fstream>

#include <stdexcept>
#include <unistd.h>

#include <map>

typedef std::map<unsigned int, unsigned long> call_cnt_map_t ;
static call_cnt_map_t call_cnt_map;

static void fun ( void *
                , const expr::eval::context& input
                , expr::eval::context& output
                )
{
  const long& x (boost::get<const long&>(input.value ("x")));
  const long& id (boost::get<const long&>(input.value ("id")));

  MLOG (DEBUG, "fun : " << id);

  ++call_cnt_map[x];

  output.bind ("done", we::type::literal::control());
}

static unsigned long call_cnt_A = 0;
static void A ( void *
              , const expr::eval::context& input
              , expr::eval::context& output
              )
{
  const long& id (boost::get<const long&>(input.value ("id")));

  MLOG (DEBUG, "A : " << id);

  ++call_cnt_A;

  output.bind ("done", pnet::type::value::value_type (we::type::literal::control()));
}

static unsigned long call_cnt_B = 0;
static void B ( void *
              , const expr::eval::context & input
              , expr::eval::context & output
              )
{
  const long& id (boost::get<const long&>(input.value("id")));

  MLOG (DEBUG, "B : " << id);

  ++call_cnt_B;

  output.bind ("done", pnet::type::value::value_type (we::type::literal::control()));
}

static unsigned long call_cnt_C = 0;
static void C ( void *
              , const expr::eval::context & input
              , expr::eval::context & output
              )
{
  const long& id (boost::get<const long&>(input.value ("id")));

  MLOG (DEBUG, "C : " << id);

  ++call_cnt_C;

  output.bind ("done", pnet::type::value::value_type (we::type::literal::control()));
}

static unsigned long call_cnt_D = 0;
static void D ( void *
              , const expr::eval::context & input
              , expr::eval::context & output
              )
{
  const long& id (boost::get<const long&>(input.value ("id")));

  MLOG (DEBUG, "D : " << id);

  ++call_cnt_D;

  output.bind ("done", pnet::type::value::value_type(we::type::literal::control()));
}

static void finalize ( void *
                     , const expr::eval::context &
                     , expr::eval::context & output
                     )
{
  for ( call_cnt_map_t::const_iterator cnt (call_cnt_map.begin())
      ; cnt != call_cnt_map.end()
      ; ++cnt
      )
    {
      MLOG (INFO, "call_cnt [ " << cnt->first << " ] = " << cnt->second);
    }

  MLOG (INFO, "call_cnt_A = " << call_cnt_A);
  MLOG (INFO, "call_cnt_B = " << call_cnt_B);
  MLOG (INFO, "call_cnt_C = " << call_cnt_C);
  MLOG (INFO, "call_cnt_D = " << call_cnt_D);

  output.bind ("done", pnet::type::value::value_type (we::type::literal::control()));
}

WE_MOD_INITIALIZE_START (concurrent);
{
  WE_REGISTER_FUN (A);
  WE_REGISTER_FUN (B);
  WE_REGISTER_FUN (C);
  WE_REGISTER_FUN (D);
  WE_REGISTER_FUN (fun);
  WE_REGISTER_FUN (finalize);
}
WE_MOD_INITIALIZE_END (concurrent);

WE_MOD_FINALIZE_START (concurrent);
{
}
WE_MOD_FINALIZE_END (concurrent);
